#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include <string.h>
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define TEST_FOLDER "tests/"

bool is_test_c(const char *path)
{
    const char *ext = ".test.c";
    size_t path_len = strlen(path);
    size_t ext_len = strlen(ext);

    return path_len >= ext_len &&
           strcmp(path + path_len - ext_len, ext) == 0;
}

bool run_tests(Nob_Walk_Entry entry)
{
    if (entry.type == NOB_FILE_REGULAR)
    {
        // if it's a .test.c file, compile it
        if (is_test_c(entry.path))
        {
            char *bin_path = strdup(entry.path);
            char *dot = strrchr(bin_path, '.');
            *dot = '\0';
            char output_path[128] = {0};
            strcat(output_path, BUILD_FOLDER);
            strcat(output_path, (const char *)bin_path);
            Cmd cmd = {0};
            nob_cc(&cmd);
            nob_cc_flags(&cmd);
            nob_cc_output(&cmd, output_path);
            nob_cc_inputs(&cmd, entry.path);
            if (!cmd_run(&cmd))
                nob_log(NOB_ERROR, "compile failed when handling with %s", entry.path);
            // run test
            nob_cmd_append(&cmd, output_path);
            if (!cmd_run(&cmd))
                nob_log(NOB_ERROR, "TEST: %s test fails.", entry.path);
            else
                nob_log(NOB_INFO, "TEST: %s test passes.", entry.path);
        }
    }
    return true;
}

void print_help(char *prog_name)
{
    printf(
        "Usage: %s [-h | --help] [-d | --debug]\n"
        "  这个程序被用于便捷地编译 soyocc 以及编译运行其测试\n"
        "  -h, --help    显示帮助信息\n"
        "  -d, --debug   编译 soyocc 二进制时附加调试信息\n"
        "\n"
        "子命令:\n"
        "  run           编译 soyocc 并运行\n"
        "  test          编译 soyocc 及其测试并运行测试\n",
        prog_name);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    bool DEBUG_MODE = false;
    bool TEST_MODE = false;
    bool RUN_MODE = false;
    if (argc != 1)
    {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            print_help(argv[0]);
            return 0;
        }
        else if (strcmp(argv[1], "--debug") == 0 || strcmp(argv[1], "-d") == 0)
            DEBUG_MODE = true;
        else if (strcmp(argv[1], "run") == 0)
            RUN_MODE = true;
        else if (strcmp(argv[1], "test") == 0)
            TEST_MODE = true;
        else
        {
            fprintf(stderr, "参数错误！\n");
            print_help(argv[0]);
            return 1;
        }
    }
    // 正常编译
    Cmd cmd = {0};

    mkdir_if_not_exists(BUILD_FOLDER);
    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    if (DEBUG_MODE)
        nob_cmd_append(&cmd, "-ggdb");
    nob_cc_output(&cmd, BUILD_FOLDER "soyocc");
    nob_cc_inputs(&cmd, SRC_FOLDER "soyocc.c");
    if (!cmd_run(&cmd))
        return 1;

    if (TEST_MODE)
    {
        mkdir_if_not_exists(TEST_FOLDER);
        mkdir_if_not_exists(BUILD_FOLDER TEST_FOLDER);
        const char *test_dir = TEST_FOLDER;
        if (!nob_walk_dir(test_dir, run_tests))
            return 1;
    }

    if (RUN_MODE)
    {
        nob_cmd_append(&cmd, BUILD_FOLDER "soyocc");
        for (int i = 2; i < argc; i++)
            nob_cmd_append(&cmd, argv[i]);
        if (!cmd_run(&cmd))
            return 1;
    }

    return 0;
}
