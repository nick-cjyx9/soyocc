#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"
#include <string.h>
#ifndef soyo_cc
#define soyo_cc(cmd) nob_cmd_append(cmd, "zig", "cc")
#endif

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define TEST_FOLDER "tests/"
#define COMPDB_PATH BUILD_FOLDER "compile_commands.json"

static const char *const SOURCES[] = {
    SRC_FOLDER "soyocc.c",
    SRC_FOLDER "tokenize.c",
    SRC_FOLDER "parse.c",
    SRC_FOLDER "codegen.c",
};

bool is_test_c(const char *path) {
  const char *ext = ".test.c";
  size_t path_len = strlen(path);
  size_t ext_len = strlen(ext);

  return path_len >= ext_len && strcmp(path + path_len - ext_len, ext) == 0;
}

bool run_tests(Nob_Walk_Entry entry) {
  if (entry.type == NOB_FILE_REGULAR) {
    // if it's a .test.c file, compile it
    if (is_test_c(entry.path)) {
      char *bin_path = strdup(entry.path);
      char *dot = strrchr(bin_path, '.');
      *dot = '\0';
      char output_path[128] = {0};
      strcat(output_path, BUILD_FOLDER);
      strcat(output_path, (const char *)bin_path);
      Cmd cmd = {0};
      soyo_cc(&cmd);
      nob_cc_flags(&cmd);
      nob_cc_output(&cmd, output_path);
      nob_cc_inputs(&cmd, entry.path);
      if (!cmd_run(&cmd))
        nob_log(NOB_ERROR, "compile failed when handling with %s", entry.path);
      // run test
      nob_cmd_append(&cmd, output_path);
      if (cmd_run(&cmd))
        nob_log(NOB_INFO, "TEST: %s passes.", entry.path);
      else {
        *(bool *)(entry.data) = false;
        nob_log(NOB_ERROR, "TEST: %s fails.", entry.path);
      }
    }
  }
  return true;
}

void print_help(char *prog_name) {
  printf("Usage: %s [-h | --help] [-d | --debug] [run | test | compdb]\n"
         "  这个程序被用于便捷地编译 soyocc 以及编译运行其测试\n"
         "  -h, --help    显示帮助信息\n"
         "  -d, --debug   编译 soyocc 二进制时附加调试信息\n"
         "\n"
         "子命令:\n"
         "  run           编译 soyocc 并运行\n"
         "  test          编译 soyocc 及其测试并运行测试\n"
         "  compdb        只生成 " COMPDB_PATH "\n",
         prog_name);
}

static void sb_append_json_string(Nob_String_Builder *sb, const char *s) {
  nob_sb_append_cstr(sb, "\"");
  for (const char *p = s; *p != '\0'; ++p) {
    switch (*p) {
    case '\"':
      nob_sb_append_cstr(sb, "\\\"");
      break;
    case '\\':
      nob_sb_append_cstr(sb, "\\\\");
      break;
    case '\n':
      nob_sb_append_cstr(sb, "\\n");
      break;
    case '\r':
      nob_sb_append_cstr(sb, "\\r");
      break;
    case '\t':
      nob_sb_append_cstr(sb, "\\t");
      break;
    default:
      nob_sb_appendf(sb, "%c", *p);
      break;
    }
  }
  nob_sb_append_cstr(sb, "\"");
}

bool write_compdb(void) {
  const char *dir = nob_get_current_dir_temp();
  if (dir == NULL) {
    nob_log(NOB_ERROR, "无法获取当前工作目录");
    return false;
  }
  if (!mkdir_if_not_exists(BUILD_FOLDER))
    return false;

  Nob_String_Builder sb = {0};
  nob_sb_append_cstr(&sb, "[\n");
  for (size_t i = 0; i < NOB_ARRAY_LEN(SOURCES); ++i) {
    Cmd cmd = {0};
    soyo_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-c", SOURCES[i]);

    nob_sb_append_cstr(&sb, "  {\n    \"directory\": ");
    sb_append_json_string(&sb, dir);
    nob_sb_append_cstr(&sb, ",\n    \"file\": ");
    sb_append_json_string(&sb, nob_temp_sprintf("%s/%s", dir, SOURCES[i]));
    nob_sb_append_cstr(&sb, ",\n    \"arguments\": [");
    for (size_t j = 0; j < cmd.count; ++j) {
      if (j > 0)
        nob_sb_append_cstr(&sb, ", ");
      sb_append_json_string(&sb, cmd.items[j]);
    }
    nob_sb_appendf(&sb, "]\n  }%s\n",
                   (i + 1 < NOB_ARRAY_LEN(SOURCES)) ? "," : "");
    nob_cmd_free(cmd);
  }
  nob_sb_append_cstr(&sb, "]\n");

  bool ok = nob_write_entire_file(COMPDB_PATH, sb.items, sb.count);
  nob_sb_free(sb);
  return ok;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  bool DEBUG_MODE = false;
  bool TEST_MODE = false;
  bool RUN_MODE = false;
  bool COMPDB_MODE = false;
  if (argc != 1) {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_help(argv[0]);
      return 0;
    } else if (strcmp(argv[1], "--debug") == 0 || strcmp(argv[1], "-d") == 0)
      DEBUG_MODE = true;
    else if (strcmp(argv[1], "run") == 0)
      RUN_MODE = true;
    else if (strcmp(argv[1], "test") == 0)
      TEST_MODE = true;
    else if (strcmp(argv[1], "compdb") == 0)
      COMPDB_MODE = true;
    else {
      fprintf(stderr, "参数错误！\n");
      print_help(argv[0]);
      return 1;
    }
  }
  if (COMPDB_MODE) {
    printf("Now generating %s...\n", COMPDB_PATH);
    return write_compdb() ? 0 : 1;
  }

  // 正常编译
  printf("Now compiling soyocc...\n");
  Cmd cmd = {0};

  mkdir_if_not_exists(BUILD_FOLDER);
  soyo_cc(&cmd);
  nob_cc_flags(&cmd);
  if (DEBUG_MODE)
    nob_cmd_append(&cmd, "-ggdb");
  nob_cc_output(&cmd, BUILD_FOLDER "soyocc");
  for (size_t i = 0; i < NOB_ARRAY_LEN(SOURCES); ++i)
    nob_cmd_append(&cmd, SOURCES[i]);
  if (!cmd_run(&cmd))
    return 1;

  if (!write_compdb())
    nob_log(NOB_WARNING, "跳过 %s 的生成", COMPDB_PATH);

  if (TEST_MODE) {
    mkdir_if_not_exists(TEST_FOLDER);
    mkdir_if_not_exists(BUILD_FOLDER TEST_FOLDER);
    printf("\nNow running tests...\n");
    bool success = true;
    const char *test_dir = TEST_FOLDER;
    if (!nob_walk_dir(test_dir, run_tests, .data = &success)) {
      nob_log(NOB_ERROR, "Failed to walk through test directory: %s", test_dir);
      return 1;
    }
    if (success) {
      printf("\nAll tests passed!\n");
      return 0;
    } else {
      printf("\nSome tests failed!\n");
      return 1;
    }
  }

  if (RUN_MODE) {
    printf("\nNow running soyocc...\n");
    nob_cmd_append(&cmd, BUILD_FOLDER "soyocc");
    for (int i = 2; i < argc; i++)
      nob_cmd_append(&cmd, argv[i]);
    if (!cmd_run(&cmd))
      return 1;
  }
  return 0;
}
