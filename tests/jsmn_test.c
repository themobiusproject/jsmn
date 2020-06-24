#include <stdlib.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdio.h>

#include "jsmn.h"

jsmn_parser p;
jsmntok_t t[500];
int total_tests = 0;
void *cur_test = NULL;

int jsmn_setup(void **state)
{
    (void)state; // unused
    jsmn_init(&p);
    memset(t, 0, 128 * sizeof(jsmntok_t));

    return 0;
}

//typedef int jsmnint_t;
//#define JSMN_NEG ((jsmnint_t)-1)

/**
 * @brief va_arg token comparison
 *
 * @param[in] s json string
 * @param[in] t pointer to jsmn tokens
 * @param[in] numtok number of tokens
 * @param[in] ap p_ap:...
 */
void vtokeq(const char *s, const jsmntok_t *t, const size_t numtok, va_list ap)
{
    if (numtok == 0)
        return;

    size_t i;
    jsmnint_t start, end, size = JSMN_NEG;
    jsmntype_t type;
    char *value = NULL;

    for (i = 0; i < numtok; i++) {
        type = va_arg(ap, int);
        switch (type) {
            case JSMN_STRING: {
                value = va_arg(ap, char *);
                size = va_arg(ap, int);
                start = end = JSMN_NEG;
                break;
            }
            case JSMN_PRIMITIVE: {
                value = va_arg(ap, char *);
                start = end = size = JSMN_NEG;
                break;
            }
            default: {
                value = NULL;
                start = va_arg(ap, int);
                end = va_arg(ap, int);
                size = va_arg(ap, int);
                break;
            }
        }
        if (!(t[i].type & type)) {
            fail_msg("token %zu type is %d, not %d", i, t[i].type, type);
        }

        if (start != JSMN_NEG && end != JSMN_NEG) {
            if (t[i].start != start) {
                fail_msg("token %zu start is %d, not %d", i, t[i].start, start);
            }
            if (t[i].end != end) {
                fail_msg("token %zu end is %d, not %d", i, t[i].end, end);
            }
        }

        if (size != JSMN_NEG && t[i].size != size) {
            fail_msg("token %zu size is %d, not %d", i, t[i].size, size);
        }

        if (s != NULL && value != NULL) {
            const char *p = s + t[i].start;
            if (strlen(value) != t[i].end - t[i].start ||
                    strncmp(p, value, t[i].end - t[i].start) != 0) {
                fail_msg("token %zu value is %.*s, not %s",
                         i, t[i].end - t[i].start, s + t[i].start, value);
            }
        }
    }
}

/**
 * @brief ...
 *
 * @param[in] s json string
 * @param[in] tokens pointer to jsmn tokens
 * @param[in] numtok number of tokens
 */
void tokeq(const char *s, const jsmntok_t *tokens, const int numtok, ...)
{
    va_list args;
    va_start(args, numtok);
    vtokeq(s, tokens, numtok, args);
    va_end(args);
}


// i_number_double_huge_neg_exp.json
static void i_number_double_huge_neg_exp(void **state)
{
    (void)state; // unused
    const char *js = "[123.456e-789]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 14, 1,
          JSMN_PRIMITIVE, "123.456e-789");
}

// i_number_huge_exp.json
static void i_number_huge_exp(void **state)
{
    (void)state; // unused
    const char *js = "[0.4e00669999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999969999999006]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 137, 1,
          JSMN_PRIMITIVE, "0.4e00669999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999969999999006");
}

// i_number_neg_int_huge_exp.json
static void i_number_neg_int_huge_exp(void **state)
{
    (void)state; // unused
    const char *js = "[-1e+9999]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_PRIMITIVE, "-1e+9999");
}

// i_number_pos_double_huge_exp.json
static void i_number_pos_double_huge_exp(void **state)
{
    (void)state; // unused
    const char *js = "[1.5e+9999]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 11, 1,
          JSMN_PRIMITIVE, "1.5e+9999");
}

// i_number_real_neg_overflow.json
static void i_number_real_neg_overflow(void **state)
{
    (void)state; // unused
    const char *js = "[-123123e100000]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_PRIMITIVE, "-123123e100000");
}

// i_number_real_pos_overflow.json
static void i_number_real_pos_overflow(void **state)
{
    (void)state; // unused
    const char *js = "[123123e100000]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 15, 1,
          JSMN_PRIMITIVE, "123123e100000");
}

// i_number_real_underflow.json
static void i_number_real_underflow(void **state)
{
    (void)state; // unused
    const char *js = "[123e-10000000]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 15, 1,
          JSMN_PRIMITIVE, "123e-10000000");
}

// i_number_too_big_neg_int.json
static void i_number_too_big_neg_int(void **state)
{
    (void)state; // unused
    const char *js = "[-123123123123123123123123123123]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 33, 1,
          JSMN_PRIMITIVE, "-123123123123123123123123123123");
}

// i_number_too_big_pos_int.json
static void i_number_too_big_pos_int(void **state)
{
    (void)state; // unused
    const char *js = "[100000000000000000000]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 23, 1,
          JSMN_PRIMITIVE, "100000000000000000000");
}

// i_number_very_big_negative_int.json
static void i_number_very_big_negative_int(void **state)
{
    (void)state; // unused
    const char *js = "[-237462374673276894279832749832423479823246327846]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 51, 1,
          JSMN_PRIMITIVE, "-237462374673276894279832749832423479823246327846");
}

// i_object_key_lone_2nd_surrogate.json
static void i_object_key_lone_2nd_surrogate(void **state)
{
    (void)state; // unused
    const char *js = "{\"\\uDFAA\":0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 12, 1,
          JSMN_STRING, "\\uDFAA", 1,
          JSMN_PRIMITIVE, "0");
}

// i_string_1st_surrogate_but_2nd_missing.json
static void i_string_1st_surrogate_but_2nd_missing(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDADA\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uDADA", 0);
}

// i_string_1st_valid_surrogate_2nd_invalid.json
static void i_string_1st_valid_surrogate_2nd_invalid(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD888\\u1234\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uD888\\u1234", 0);
}

// i_string_incomplete_surrogate_and_escape_valid.json
static void i_string_incomplete_surrogate_and_escape_valid(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\n\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 12, 1,
          JSMN_STRING, "\\uD800\\n", 0);
}

// i_string_incomplete_surrogate_pair.json
static void i_string_incomplete_surrogate_pair(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDd1ea\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 11, 1,
          JSMN_STRING, "\\uDd1ea", 0);
}

// i_string_incomplete_surrogates_escape_valid.json
static void i_string_incomplete_surrogates_escape_valid(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\uD800\\n\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 18, 1,
          JSMN_STRING, "\\uD800\\uD800\\n", 0);
}

// i_string_invalid_lonely_surrogate.json
static void i_string_invalid_lonely_surrogate(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\ud800\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\ud800", 0);
}

// i_string_invalid_surrogate.json
static void i_string_invalid_surrogate(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\ud800abc\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 13, 1,
          JSMN_STRING, "\\ud800abc", 0);
}

// i_string_invalid_utf-8.json
static void i_string_invalid_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"ˇ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_STRING, "ˇ", 0);
}

// i_string_inverted_surrogates_U+1D11E.json
static void i_string_inverted_surrogates_U_1D11E(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDd1e\\uD834\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uDd1e\\uD834", 0);
}

// i_string_iso_latin_1.json
static void i_string_iso_latin_1(void **state)
{
    (void)state; // unused
    const char *js = "[\"È\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_STRING, "È", 0);
}

// i_string_lone_second_surrogate.json
static void i_string_lone_second_surrogate(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDFAA\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uDFAA", 0);
}

// i_string_lone_utf8_continuation_byte.json
static void i_string_lone_utf8_continuation_byte(void **state)
{
    (void)state; // unused
    const char *js = "[\"Å\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_STRING, "Å", 0);
}

// i_string_not_in_unicode_range.json
static void i_string_not_in_unicode_range(void **state)
{
    (void)state; // unused
    const char *js = "[\"Ùøøø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_STRING, "Ùøøø", 0);
}

// i_string_overlong_sequence_2_bytes.json
static void i_string_overlong_sequence_2_bytes(void **state)
{
    (void)state; // unused
    const char *js = "[\"¿Ø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_STRING, "¿Ø", 0);
}

// i_string_overlong_sequence_6_bytes.json
static void i_string_overlong_sequence_6_bytes(void **state)
{
    (void)state; // unused
    const char *js = "[\"¸Éøøøø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "¸Éøøøø", 0);
}

// i_string_overlong_sequence_6_bytes_null.json
static void i_string_overlong_sequence_6_bytes_null(void **state)
{
    (void)state; // unused
    const char *js = "[\"¸ÄÄÄÄÄ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "¸ÄÄÄÄÄ", 0);
}

// i_string_truncated-utf-8.json
static void i_string_truncated_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"‡ˇ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_STRING, "‡ˇ", 0);
}

  // i_string_utf16BE_no_BOM.json failed to parse.
  // i_string_utf16LE_no_BOM.json failed to parse.
  // i_string_UTF-16LE_with_BOM.json failed to parse.
// i_string_UTF-8_invalid_sequence.json
static void i_string_UTF8_invalid_sequence(void **state)
{
    (void)state; // unused
    const char *js = "[\"Êó•—à˙\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "Êó•—à˙", 0);
}

// i_string_UTF8_surrogate_U+D800.json
static void i_string_UTF8_surrogate_U_D800(void **state)
{
    (void)state; // unused
    const char *js = "[\"Ì†Ä\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "Ì†Ä", 0);
}

// i_structure_500_nested_arrays.json
static void i_structure_500_nested_arrays(void **state)
{
    (void)state; // unused
    const char *js = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 500), 500);
    tokeq(js, t, 500,
          JSMN_ARRAY,    0,1000, 1, JSMN_ARRAY,    1, 999, 1, JSMN_ARRAY,    2, 998, 1, JSMN_ARRAY,    3, 997, 1, JSMN_ARRAY,    4, 996, 1,
          JSMN_ARRAY,    5, 995, 1, JSMN_ARRAY,    6, 994, 1, JSMN_ARRAY,    7, 993, 1, JSMN_ARRAY,    8, 992, 1, JSMN_ARRAY,    9, 991, 1,
          JSMN_ARRAY,   10, 990, 1, JSMN_ARRAY,   11, 989, 1, JSMN_ARRAY,   12, 988, 1, JSMN_ARRAY,   13, 987, 1, JSMN_ARRAY,   14, 986, 1,
          JSMN_ARRAY,   15, 985, 1, JSMN_ARRAY,   16, 984, 1, JSMN_ARRAY,   17, 983, 1, JSMN_ARRAY,   18, 982, 1, JSMN_ARRAY,   19, 981, 1,
          JSMN_ARRAY,   20, 980, 1, JSMN_ARRAY,   21, 979, 1, JSMN_ARRAY,   22, 978, 1, JSMN_ARRAY,   23, 977, 1, JSMN_ARRAY,   24, 976, 1,
          JSMN_ARRAY,   25, 975, 1, JSMN_ARRAY,   26, 974, 1, JSMN_ARRAY,   27, 973, 1, JSMN_ARRAY,   28, 972, 1, JSMN_ARRAY,   29, 971, 1,
          JSMN_ARRAY,   30, 970, 1, JSMN_ARRAY,   31, 969, 1, JSMN_ARRAY,   32, 968, 1, JSMN_ARRAY,   33, 967, 1, JSMN_ARRAY,   34, 966, 1,
          JSMN_ARRAY,   35, 965, 1, JSMN_ARRAY,   36, 964, 1, JSMN_ARRAY,   37, 963, 1, JSMN_ARRAY,   38, 962, 1, JSMN_ARRAY,   39, 961, 1,
          JSMN_ARRAY,   40, 960, 1, JSMN_ARRAY,   41, 959, 1, JSMN_ARRAY,   42, 958, 1, JSMN_ARRAY,   43, 957, 1, JSMN_ARRAY,   44, 956, 1,
          JSMN_ARRAY,   45, 955, 1, JSMN_ARRAY,   46, 954, 1, JSMN_ARRAY,   47, 953, 1, JSMN_ARRAY,   48, 952, 1, JSMN_ARRAY,   49, 951, 1,
          JSMN_ARRAY,   50, 950, 1, JSMN_ARRAY,   51, 949, 1, JSMN_ARRAY,   52, 948, 1, JSMN_ARRAY,   53, 947, 1, JSMN_ARRAY,   54, 946, 1,
          JSMN_ARRAY,   55, 945, 1, JSMN_ARRAY,   56, 944, 1, JSMN_ARRAY,   57, 943, 1, JSMN_ARRAY,   58, 942, 1, JSMN_ARRAY,   59, 941, 1,
          JSMN_ARRAY,   60, 940, 1, JSMN_ARRAY,   61, 939, 1, JSMN_ARRAY,   62, 938, 1, JSMN_ARRAY,   63, 937, 1, JSMN_ARRAY,   64, 936, 1,
          JSMN_ARRAY,   65, 935, 1, JSMN_ARRAY,   66, 934, 1, JSMN_ARRAY,   67, 933, 1, JSMN_ARRAY,   68, 932, 1, JSMN_ARRAY,   69, 931, 1,
          JSMN_ARRAY,   70, 930, 1, JSMN_ARRAY,   71, 929, 1, JSMN_ARRAY,   72, 928, 1, JSMN_ARRAY,   73, 927, 1, JSMN_ARRAY,   74, 926, 1,
          JSMN_ARRAY,   75, 925, 1, JSMN_ARRAY,   76, 924, 1, JSMN_ARRAY,   77, 923, 1, JSMN_ARRAY,   78, 922, 1, JSMN_ARRAY,   79, 921, 1,
          JSMN_ARRAY,   80, 920, 1, JSMN_ARRAY,   81, 919, 1, JSMN_ARRAY,   82, 918, 1, JSMN_ARRAY,   83, 917, 1, JSMN_ARRAY,   84, 916, 1,
          JSMN_ARRAY,   85, 915, 1, JSMN_ARRAY,   86, 914, 1, JSMN_ARRAY,   87, 913, 1, JSMN_ARRAY,   88, 912, 1, JSMN_ARRAY,   89, 911, 1,
          JSMN_ARRAY,   90, 910, 1, JSMN_ARRAY,   91, 909, 1, JSMN_ARRAY,   92, 908, 1, JSMN_ARRAY,   93, 907, 1, JSMN_ARRAY,   94, 906, 1,
          JSMN_ARRAY,   95, 905, 1, JSMN_ARRAY,   96, 904, 1, JSMN_ARRAY,   97, 903, 1, JSMN_ARRAY,   98, 902, 1, JSMN_ARRAY,   99, 901, 1,
          JSMN_ARRAY,  100, 900, 1, JSMN_ARRAY,  101, 899, 1, JSMN_ARRAY,  102, 898, 1, JSMN_ARRAY,  103, 897, 1, JSMN_ARRAY,  104, 896, 1,
          JSMN_ARRAY,  105, 895, 1, JSMN_ARRAY,  106, 894, 1, JSMN_ARRAY,  107, 893, 1, JSMN_ARRAY,  108, 892, 1, JSMN_ARRAY,  109, 891, 1,
          JSMN_ARRAY,  110, 890, 1, JSMN_ARRAY,  111, 889, 1, JSMN_ARRAY,  112, 888, 1, JSMN_ARRAY,  113, 887, 1, JSMN_ARRAY,  114, 886, 1,
          JSMN_ARRAY,  115, 885, 1, JSMN_ARRAY,  116, 884, 1, JSMN_ARRAY,  117, 883, 1, JSMN_ARRAY,  118, 882, 1, JSMN_ARRAY,  119, 881, 1,
          JSMN_ARRAY,  120, 880, 1, JSMN_ARRAY,  121, 879, 1, JSMN_ARRAY,  122, 878, 1, JSMN_ARRAY,  123, 877, 1, JSMN_ARRAY,  124, 876, 1,
          JSMN_ARRAY,  125, 875, 1, JSMN_ARRAY,  126, 874, 1, JSMN_ARRAY,  127, 873, 1, JSMN_ARRAY,  128, 872, 1, JSMN_ARRAY,  129, 871, 1,
          JSMN_ARRAY,  130, 870, 1, JSMN_ARRAY,  131, 869, 1, JSMN_ARRAY,  132, 868, 1, JSMN_ARRAY,  133, 867, 1, JSMN_ARRAY,  134, 866, 1,
          JSMN_ARRAY,  135, 865, 1, JSMN_ARRAY,  136, 864, 1, JSMN_ARRAY,  137, 863, 1, JSMN_ARRAY,  138, 862, 1, JSMN_ARRAY,  139, 861, 1,
          JSMN_ARRAY,  140, 860, 1, JSMN_ARRAY,  141, 859, 1, JSMN_ARRAY,  142, 858, 1, JSMN_ARRAY,  143, 857, 1, JSMN_ARRAY,  144, 856, 1,
          JSMN_ARRAY,  145, 855, 1, JSMN_ARRAY,  146, 854, 1, JSMN_ARRAY,  147, 853, 1, JSMN_ARRAY,  148, 852, 1, JSMN_ARRAY,  149, 851, 1,
          JSMN_ARRAY,  150, 850, 1, JSMN_ARRAY,  151, 849, 1, JSMN_ARRAY,  152, 848, 1, JSMN_ARRAY,  153, 847, 1, JSMN_ARRAY,  154, 846, 1,
          JSMN_ARRAY,  155, 845, 1, JSMN_ARRAY,  156, 844, 1, JSMN_ARRAY,  157, 843, 1, JSMN_ARRAY,  158, 842, 1, JSMN_ARRAY,  159, 841, 1,
          JSMN_ARRAY,  160, 840, 1, JSMN_ARRAY,  161, 839, 1, JSMN_ARRAY,  162, 838, 1, JSMN_ARRAY,  163, 837, 1, JSMN_ARRAY,  164, 836, 1,
          JSMN_ARRAY,  165, 835, 1, JSMN_ARRAY,  166, 834, 1, JSMN_ARRAY,  167, 833, 1, JSMN_ARRAY,  168, 832, 1, JSMN_ARRAY,  169, 831, 1,
          JSMN_ARRAY,  170, 830, 1, JSMN_ARRAY,  171, 829, 1, JSMN_ARRAY,  172, 828, 1, JSMN_ARRAY,  173, 827, 1, JSMN_ARRAY,  174, 826, 1,
          JSMN_ARRAY,  175, 825, 1, JSMN_ARRAY,  176, 824, 1, JSMN_ARRAY,  177, 823, 1, JSMN_ARRAY,  178, 822, 1, JSMN_ARRAY,  179, 821, 1,
          JSMN_ARRAY,  180, 820, 1, JSMN_ARRAY,  181, 819, 1, JSMN_ARRAY,  182, 818, 1, JSMN_ARRAY,  183, 817, 1, JSMN_ARRAY,  184, 816, 1,
          JSMN_ARRAY,  185, 815, 1, JSMN_ARRAY,  186, 814, 1, JSMN_ARRAY,  187, 813, 1, JSMN_ARRAY,  188, 812, 1, JSMN_ARRAY,  189, 811, 1,
          JSMN_ARRAY,  190, 810, 1, JSMN_ARRAY,  191, 809, 1, JSMN_ARRAY,  192, 808, 1, JSMN_ARRAY,  193, 807, 1, JSMN_ARRAY,  194, 806, 1,
          JSMN_ARRAY,  195, 805, 1, JSMN_ARRAY,  196, 804, 1, JSMN_ARRAY,  197, 803, 1, JSMN_ARRAY,  198, 802, 1, JSMN_ARRAY,  199, 801, 1,
          JSMN_ARRAY,  200, 800, 1, JSMN_ARRAY,  201, 799, 1, JSMN_ARRAY,  202, 798, 1, JSMN_ARRAY,  203, 797, 1, JSMN_ARRAY,  204, 796, 1,
          JSMN_ARRAY,  205, 795, 1, JSMN_ARRAY,  206, 794, 1, JSMN_ARRAY,  207, 793, 1, JSMN_ARRAY,  208, 792, 1, JSMN_ARRAY,  209, 791, 1,
          JSMN_ARRAY,  210, 790, 1, JSMN_ARRAY,  211, 789, 1, JSMN_ARRAY,  212, 788, 1, JSMN_ARRAY,  213, 787, 1, JSMN_ARRAY,  214, 786, 1,
          JSMN_ARRAY,  215, 785, 1, JSMN_ARRAY,  216, 784, 1, JSMN_ARRAY,  217, 783, 1, JSMN_ARRAY,  218, 782, 1, JSMN_ARRAY,  219, 781, 1,
          JSMN_ARRAY,  220, 780, 1, JSMN_ARRAY,  221, 779, 1, JSMN_ARRAY,  222, 778, 1, JSMN_ARRAY,  223, 777, 1, JSMN_ARRAY,  224, 776, 1,
          JSMN_ARRAY,  225, 775, 1, JSMN_ARRAY,  226, 774, 1, JSMN_ARRAY,  227, 773, 1, JSMN_ARRAY,  228, 772, 1, JSMN_ARRAY,  229, 771, 1,
          JSMN_ARRAY,  230, 770, 1, JSMN_ARRAY,  231, 769, 1, JSMN_ARRAY,  232, 768, 1, JSMN_ARRAY,  233, 767, 1, JSMN_ARRAY,  234, 766, 1,
          JSMN_ARRAY,  235, 765, 1, JSMN_ARRAY,  236, 764, 1, JSMN_ARRAY,  237, 763, 1, JSMN_ARRAY,  238, 762, 1, JSMN_ARRAY,  239, 761, 1,
          JSMN_ARRAY,  240, 760, 1, JSMN_ARRAY,  241, 759, 1, JSMN_ARRAY,  242, 758, 1, JSMN_ARRAY,  243, 757, 1, JSMN_ARRAY,  244, 756, 1,
          JSMN_ARRAY,  245, 755, 1, JSMN_ARRAY,  246, 754, 1, JSMN_ARRAY,  247, 753, 1, JSMN_ARRAY,  248, 752, 1, JSMN_ARRAY,  249, 751, 1,
          JSMN_ARRAY,  250, 750, 1, JSMN_ARRAY,  251, 749, 1, JSMN_ARRAY,  252, 748, 1, JSMN_ARRAY,  253, 747, 1, JSMN_ARRAY,  254, 746, 1,
          JSMN_ARRAY,  255, 745, 1, JSMN_ARRAY,  256, 744, 1, JSMN_ARRAY,  257, 743, 1, JSMN_ARRAY,  258, 742, 1, JSMN_ARRAY,  259, 741, 1,
          JSMN_ARRAY,  260, 740, 1, JSMN_ARRAY,  261, 739, 1, JSMN_ARRAY,  262, 738, 1, JSMN_ARRAY,  263, 737, 1, JSMN_ARRAY,  264, 736, 1,
          JSMN_ARRAY,  265, 735, 1, JSMN_ARRAY,  266, 734, 1, JSMN_ARRAY,  267, 733, 1, JSMN_ARRAY,  268, 732, 1, JSMN_ARRAY,  269, 731, 1,
          JSMN_ARRAY,  270, 730, 1, JSMN_ARRAY,  271, 729, 1, JSMN_ARRAY,  272, 728, 1, JSMN_ARRAY,  273, 727, 1, JSMN_ARRAY,  274, 726, 1,
          JSMN_ARRAY,  275, 725, 1, JSMN_ARRAY,  276, 724, 1, JSMN_ARRAY,  277, 723, 1, JSMN_ARRAY,  278, 722, 1, JSMN_ARRAY,  279, 721, 1,
          JSMN_ARRAY,  280, 720, 1, JSMN_ARRAY,  281, 719, 1, JSMN_ARRAY,  282, 718, 1, JSMN_ARRAY,  283, 717, 1, JSMN_ARRAY,  284, 716, 1,
          JSMN_ARRAY,  285, 715, 1, JSMN_ARRAY,  286, 714, 1, JSMN_ARRAY,  287, 713, 1, JSMN_ARRAY,  288, 712, 1, JSMN_ARRAY,  289, 711, 1,
          JSMN_ARRAY,  290, 710, 1, JSMN_ARRAY,  291, 709, 1, JSMN_ARRAY,  292, 708, 1, JSMN_ARRAY,  293, 707, 1, JSMN_ARRAY,  294, 706, 1,
          JSMN_ARRAY,  295, 705, 1, JSMN_ARRAY,  296, 704, 1, JSMN_ARRAY,  297, 703, 1, JSMN_ARRAY,  298, 702, 1, JSMN_ARRAY,  299, 701, 1,
          JSMN_ARRAY,  300, 700, 1, JSMN_ARRAY,  301, 699, 1, JSMN_ARRAY,  302, 698, 1, JSMN_ARRAY,  303, 697, 1, JSMN_ARRAY,  304, 696, 1,
          JSMN_ARRAY,  305, 695, 1, JSMN_ARRAY,  306, 694, 1, JSMN_ARRAY,  307, 693, 1, JSMN_ARRAY,  308, 692, 1, JSMN_ARRAY,  309, 691, 1,
          JSMN_ARRAY,  310, 690, 1, JSMN_ARRAY,  311, 689, 1, JSMN_ARRAY,  312, 688, 1, JSMN_ARRAY,  313, 687, 1, JSMN_ARRAY,  314, 686, 1,
          JSMN_ARRAY,  315, 685, 1, JSMN_ARRAY,  316, 684, 1, JSMN_ARRAY,  317, 683, 1, JSMN_ARRAY,  318, 682, 1, JSMN_ARRAY,  319, 681, 1,
          JSMN_ARRAY,  320, 680, 1, JSMN_ARRAY,  321, 679, 1, JSMN_ARRAY,  322, 678, 1, JSMN_ARRAY,  323, 677, 1, JSMN_ARRAY,  324, 676, 1,
          JSMN_ARRAY,  325, 675, 1, JSMN_ARRAY,  326, 674, 1, JSMN_ARRAY,  327, 673, 1, JSMN_ARRAY,  328, 672, 1, JSMN_ARRAY,  329, 671, 1,
          JSMN_ARRAY,  330, 670, 1, JSMN_ARRAY,  331, 669, 1, JSMN_ARRAY,  332, 668, 1, JSMN_ARRAY,  333, 667, 1, JSMN_ARRAY,  334, 666, 1,
          JSMN_ARRAY,  335, 665, 1, JSMN_ARRAY,  336, 664, 1, JSMN_ARRAY,  337, 663, 1, JSMN_ARRAY,  338, 662, 1, JSMN_ARRAY,  339, 661, 1,
          JSMN_ARRAY,  340, 660, 1, JSMN_ARRAY,  341, 659, 1, JSMN_ARRAY,  342, 658, 1, JSMN_ARRAY,  343, 657, 1, JSMN_ARRAY,  344, 656, 1,
          JSMN_ARRAY,  345, 655, 1, JSMN_ARRAY,  346, 654, 1, JSMN_ARRAY,  347, 653, 1, JSMN_ARRAY,  348, 652, 1, JSMN_ARRAY,  349, 651, 1,
          JSMN_ARRAY,  350, 650, 1, JSMN_ARRAY,  351, 649, 1, JSMN_ARRAY,  352, 648, 1, JSMN_ARRAY,  353, 647, 1, JSMN_ARRAY,  354, 646, 1,
          JSMN_ARRAY,  355, 645, 1, JSMN_ARRAY,  356, 644, 1, JSMN_ARRAY,  357, 643, 1, JSMN_ARRAY,  358, 642, 1, JSMN_ARRAY,  359, 641, 1,
          JSMN_ARRAY,  360, 640, 1, JSMN_ARRAY,  361, 639, 1, JSMN_ARRAY,  362, 638, 1, JSMN_ARRAY,  363, 637, 1, JSMN_ARRAY,  364, 636, 1,
          JSMN_ARRAY,  365, 635, 1, JSMN_ARRAY,  366, 634, 1, JSMN_ARRAY,  367, 633, 1, JSMN_ARRAY,  368, 632, 1, JSMN_ARRAY,  369, 631, 1,
          JSMN_ARRAY,  370, 630, 1, JSMN_ARRAY,  371, 629, 1, JSMN_ARRAY,  372, 628, 1, JSMN_ARRAY,  373, 627, 1, JSMN_ARRAY,  374, 626, 1,
          JSMN_ARRAY,  375, 625, 1, JSMN_ARRAY,  376, 624, 1, JSMN_ARRAY,  377, 623, 1, JSMN_ARRAY,  378, 622, 1, JSMN_ARRAY,  379, 621, 1,
          JSMN_ARRAY,  380, 620, 1, JSMN_ARRAY,  381, 619, 1, JSMN_ARRAY,  382, 618, 1, JSMN_ARRAY,  383, 617, 1, JSMN_ARRAY,  384, 616, 1,
          JSMN_ARRAY,  385, 615, 1, JSMN_ARRAY,  386, 614, 1, JSMN_ARRAY,  387, 613, 1, JSMN_ARRAY,  388, 612, 1, JSMN_ARRAY,  389, 611, 1,
          JSMN_ARRAY,  390, 610, 1, JSMN_ARRAY,  391, 609, 1, JSMN_ARRAY,  392, 608, 1, JSMN_ARRAY,  393, 607, 1, JSMN_ARRAY,  394, 606, 1,
          JSMN_ARRAY,  395, 605, 1, JSMN_ARRAY,  396, 604, 1, JSMN_ARRAY,  397, 603, 1, JSMN_ARRAY,  398, 602, 1, JSMN_ARRAY,  399, 601, 1,
          JSMN_ARRAY,  400, 600, 1, JSMN_ARRAY,  401, 599, 1, JSMN_ARRAY,  402, 598, 1, JSMN_ARRAY,  403, 597, 1, JSMN_ARRAY,  404, 596, 1,
          JSMN_ARRAY,  405, 595, 1, JSMN_ARRAY,  406, 594, 1, JSMN_ARRAY,  407, 593, 1, JSMN_ARRAY,  408, 592, 1, JSMN_ARRAY,  409, 591, 1,
          JSMN_ARRAY,  410, 590, 1, JSMN_ARRAY,  411, 589, 1, JSMN_ARRAY,  412, 588, 1, JSMN_ARRAY,  413, 587, 1, JSMN_ARRAY,  414, 586, 1,
          JSMN_ARRAY,  415, 585, 1, JSMN_ARRAY,  416, 584, 1, JSMN_ARRAY,  417, 583, 1, JSMN_ARRAY,  418, 582, 1, JSMN_ARRAY,  419, 581, 1,
          JSMN_ARRAY,  420, 580, 1, JSMN_ARRAY,  421, 579, 1, JSMN_ARRAY,  422, 578, 1, JSMN_ARRAY,  423, 577, 1, JSMN_ARRAY,  424, 576, 1,
          JSMN_ARRAY,  425, 575, 1, JSMN_ARRAY,  426, 574, 1, JSMN_ARRAY,  427, 573, 1, JSMN_ARRAY,  428, 572, 1, JSMN_ARRAY,  429, 571, 1,
          JSMN_ARRAY,  430, 570, 1, JSMN_ARRAY,  431, 569, 1, JSMN_ARRAY,  432, 568, 1, JSMN_ARRAY,  433, 567, 1, JSMN_ARRAY,  434, 566, 1,
          JSMN_ARRAY,  435, 565, 1, JSMN_ARRAY,  436, 564, 1, JSMN_ARRAY,  437, 563, 1, JSMN_ARRAY,  438, 562, 1, JSMN_ARRAY,  439, 561, 1,
          JSMN_ARRAY,  440, 560, 1, JSMN_ARRAY,  441, 559, 1, JSMN_ARRAY,  442, 558, 1, JSMN_ARRAY,  443, 557, 1, JSMN_ARRAY,  444, 556, 1,
          JSMN_ARRAY,  445, 555, 1, JSMN_ARRAY,  446, 554, 1, JSMN_ARRAY,  447, 553, 1, JSMN_ARRAY,  448, 552, 1, JSMN_ARRAY,  449, 551, 1,
          JSMN_ARRAY,  450, 550, 1, JSMN_ARRAY,  451, 549, 1, JSMN_ARRAY,  452, 548, 1, JSMN_ARRAY,  453, 547, 1, JSMN_ARRAY,  454, 546, 1,
          JSMN_ARRAY,  455, 545, 1, JSMN_ARRAY,  456, 544, 1, JSMN_ARRAY,  457, 543, 1, JSMN_ARRAY,  458, 542, 1, JSMN_ARRAY,  459, 541, 1,
          JSMN_ARRAY,  460, 540, 1, JSMN_ARRAY,  461, 539, 1, JSMN_ARRAY,  462, 538, 1, JSMN_ARRAY,  463, 537, 1, JSMN_ARRAY,  464, 536, 1,
          JSMN_ARRAY,  465, 535, 1, JSMN_ARRAY,  466, 534, 1, JSMN_ARRAY,  467, 533, 1, JSMN_ARRAY,  468, 532, 1, JSMN_ARRAY,  469, 531, 1,
          JSMN_ARRAY,  470, 530, 1, JSMN_ARRAY,  471, 529, 1, JSMN_ARRAY,  472, 528, 1, JSMN_ARRAY,  473, 527, 1, JSMN_ARRAY,  474, 526, 1,
          JSMN_ARRAY,  475, 525, 1, JSMN_ARRAY,  476, 524, 1, JSMN_ARRAY,  477, 523, 1, JSMN_ARRAY,  478, 522, 1, JSMN_ARRAY,  479, 521, 1,
          JSMN_ARRAY,  480, 520, 1, JSMN_ARRAY,  481, 519, 1, JSMN_ARRAY,  482, 518, 1, JSMN_ARRAY,  483, 517, 1, JSMN_ARRAY,  484, 516, 1,
          JSMN_ARRAY,  485, 515, 1, JSMN_ARRAY,  486, 514, 1, JSMN_ARRAY,  487, 513, 1, JSMN_ARRAY,  488, 512, 1, JSMN_ARRAY,  489, 511, 1,
          JSMN_ARRAY,  490, 510, 1, JSMN_ARRAY,  491, 509, 1, JSMN_ARRAY,  492, 508, 1, JSMN_ARRAY,  493, 507, 1, JSMN_ARRAY,  494, 506, 1,
          JSMN_ARRAY,  495, 505, 1, JSMN_ARRAY,  496, 504, 1, JSMN_ARRAY,  497, 503, 1, JSMN_ARRAY,  498, 502, 1, JSMN_ARRAY,  499, 501, 0);
}

// i_structure_UTF-8_BOM_empty_object.json failed to parse.

void test_jsontestsuite_i(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(i_number_double_huge_neg_exp, jsmn_setup),
        cmocka_unit_test_setup(i_number_huge_exp, jsmn_setup),
        cmocka_unit_test_setup(i_number_neg_int_huge_exp, jsmn_setup),
        cmocka_unit_test_setup(i_number_pos_double_huge_exp, jsmn_setup),
        cmocka_unit_test_setup(i_number_real_neg_overflow, jsmn_setup),
        cmocka_unit_test_setup(i_number_real_pos_overflow, jsmn_setup),
        cmocka_unit_test_setup(i_number_real_underflow, jsmn_setup),
        cmocka_unit_test_setup(i_number_too_big_neg_int, jsmn_setup),
        cmocka_unit_test_setup(i_number_too_big_pos_int, jsmn_setup),
        cmocka_unit_test_setup(i_number_very_big_negative_int, jsmn_setup),
        cmocka_unit_test_setup(i_object_key_lone_2nd_surrogate, jsmn_setup),
        cmocka_unit_test_setup(i_string_1st_surrogate_but_2nd_missing, jsmn_setup),
        cmocka_unit_test_setup(i_string_1st_valid_surrogate_2nd_invalid, jsmn_setup),
        cmocka_unit_test_setup(i_string_incomplete_surrogate_and_escape_valid, jsmn_setup),
        cmocka_unit_test_setup(i_string_incomplete_surrogate_pair, jsmn_setup),
        cmocka_unit_test_setup(i_string_incomplete_surrogates_escape_valid, jsmn_setup),
        cmocka_unit_test_setup(i_string_invalid_lonely_surrogate, jsmn_setup),
        cmocka_unit_test_setup(i_string_invalid_surrogate, jsmn_setup),
        cmocka_unit_test_setup(i_string_invalid_utf8, jsmn_setup),
        cmocka_unit_test_setup(i_string_inverted_surrogates_U_1D11E, jsmn_setup),
        cmocka_unit_test_setup(i_string_iso_latin_1, jsmn_setup),
        cmocka_unit_test_setup(i_string_lone_second_surrogate, jsmn_setup),
        cmocka_unit_test_setup(i_string_lone_utf8_continuation_byte, jsmn_setup),
        cmocka_unit_test_setup(i_string_not_in_unicode_range, jsmn_setup),
        cmocka_unit_test_setup(i_string_overlong_sequence_2_bytes, jsmn_setup),
        cmocka_unit_test_setup(i_string_overlong_sequence_6_bytes, jsmn_setup),
        cmocka_unit_test_setup(i_string_overlong_sequence_6_bytes_null, jsmn_setup),
        cmocka_unit_test_setup(i_string_truncated_utf8, jsmn_setup),
//      cmocka_unit_test_setup(i_string_UTF16LE_with_BOM, jsmn_setup),
        cmocka_unit_test_setup(i_string_UTF8_invalid_sequence, jsmn_setup),
//      cmocka_unit_test_setup(i_string_UTF16BE_no_BOM, jsmn_setup),
//      cmocka_unit_test_setup(i_string_UTF16LE_no_BOM, jsmn_setup),
        cmocka_unit_test_setup(i_string_UTF8_surrogate_U_D800, jsmn_setup),
        cmocka_unit_test_setup(i_structure_500_nested_arrays, jsmn_setup),
//      cmocka_unit_test_setup(i_structure_UTF8_BOM_empty_object, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("JSONTestSuite tests that may pass depending on implementation.", tests, NULL, NULL);
}

// n_array_1_true_without_comma.json
static void n_array_1_true_without_comma(void **state)
{
    (void)state; // unused
    const char *js = "[1 true]";
#ifndef JSMN_PERMISSIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_array_a_invalid_utf8.json
static void n_array_a_invalid_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[aÂ]";
#if !defined(JSMN_PERMISSIVE_PRIMITIVE) || !defined(JSMN_UTF8)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_array_colon_instead_of_comma.json
static void n_array_colon_instead_of_comma(void **state)
{
    (void)state; // unused
    const char *js = "[\"\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_comma_after_close.json
static void n_array_comma_after_close(void **state)
{
    (void)state; // unused
    const char *js = "[\"\"],";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_array_comma_and_number.json
static void n_array_comma_and_number(void **state)
{
    (void)state; // unused
    const char *js = "[,1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_double_comma.json
static void n_array_double_comma(void **state)
{
    (void)state; // unused
    const char *js = "[1,,2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_double_extra_comma.json
static void n_array_double_extra_comma(void **state)
{
    (void)state; // unused
    const char *js = "[\"x\",,]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_extra_close.json
static void n_array_extra_close(void **state)
{
    (void)state; // unused
    const char *js = "[\"x\"]]";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_array_extra_comma.json
static void n_array_extra_comma(void **state)
{
    (void)state; // unused
    const char *js = "[\"\",]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_incomplete_invalid_value.json
static void n_array_incomplete_invalid_value(void **state)
{
    (void)state; // unused
    const char *js = "[x";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_incomplete.json
static void n_array_incomplete(void **state)
{
    (void)state; // unused
    const char *js = "[\"x\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_array_inner_array_no_comma.json
static void n_array_inner_array_no_comma(void **state)
{
    (void)state; // unused
    const char *js = "[3[4]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_invalid_utf8.json
static void n_array_invalid_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[ˇ]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_items_separated_by_semicolon.json
static void n_array_items_separated_by_semicolon(void **state)
{
    (void)state; // unused
    const char *js = "[1:2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_just_comma.json
static void n_array_just_comma(void **state)
{
    (void)state; // unused
    const char *js = "[,]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_just_minus.json
static void n_array_just_minus(void **state)
{
    (void)state; // unused
    const char *js = "[-]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_array_missing_value.json
static void n_array_missing_value(void **state)
{
    (void)state; // unused
    const char *js = "[   , \"\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_newlines_unclosed.json
static void n_array_newlines_unclosed(void **state)
{
    (void)state; // unused
    const char *js = "[\"a\",\n4\n,1,";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_array_number_and_comma.json
static void n_array_number_and_comma(void **state)
{
    (void)state; // unused
    const char *js = "[1,]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_number_and_several_commas.json
static void n_array_number_and_several_commas(void **state)
{
    (void)state; // unused
    const char *js = "[1,,]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_spaces_vertical_tab_formfeed.json
static void n_array_spaces_vertical_tab_formfeed(void **state)
{
    (void)state; // unused
    const char *js = "[\"\va\"\f]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_star_inside.json
static void n_array_star_inside(void **state)
{
    (void)state; // unused
    const char *js = "[*]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_array_unclosed.json
static void n_array_unclosed(void **state)
{
    (void)state; // unused
    const char *js = "[\"\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_array_unclosed_trailing_comma.json
static void n_array_unclosed_trailing_comma(void **state)
{
    (void)state; // unused
    const char *js = "[1,";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_array_unclosed_with_new_lines.json
static void n_array_unclosed_with_new_lines(void **state)
{
    (void)state; // unused
    const char *js = "[1,\n1\n,1";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_array_unclosed_with_object_inside.json
static void n_array_unclosed_with_object_inside(void **state)
{
    (void)state; // unused
    const char *js = "[{}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_incomplete_false.json
static void n_incomplete_false(void **state)
{
    (void)state; // unused
    const char *js = "[fals]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_incomplete_null.json
static void n_incomplete_null(void **state)
{
    (void)state; // unused
    const char *js = "[nul]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_incomplete_true.json
static void n_incomplete_true(void **state)
{
    (void)state; // unused
    const char *js = "[tru]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_multidigit_number_then_00.json
static void n_multidigit_number_then_00(void **state)
{
    (void)state; // unused
    const char *js = "123\0";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, 4, NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, 4, NULL, 0), 1);
#endif
}

// n_number_0.1.2.json
static void n_number_0dot1dot2(void **state)
{
    (void)state; // unused
    const char *js = "[0.1.2]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_-01.json
static void n_number_minus01(void **state)
{
    (void)state; // unused
    const char *js = "[-01]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0.3e+.json
static void n_number_0dot3_eplus(void **state)
{
    (void)state; // unused
    const char *js = "[0.3e+]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0.3e.json
static void n_number_0dot3_e(void **state)
{
    (void)state; // unused
    const char *js = "[0.3e]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0_capital_E+.json
static void n_number_0_capital_Eplus(void **state)
{
    (void)state; // unused
    const char *js = "[0E+]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0_capital_E.json
static void n_number_0_capital_E(void **state)
{
    (void)state; // unused
    const char *js = "[0E]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0.e1.json
static void n_number_0dot_e1(void **state)
{
    (void)state; // unused
    const char *js = "[0.e1]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0e+.json
static void n_number_0_eplus(void **state)
{
    (void)state; // unused
    const char *js = "[0e+]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_0e.json
static void n_number_0_e(void **state)
{
    (void)state; // unused
    const char *js = "[0e]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_1_000.json
static void n_number_1_000(void **state)
{
    (void)state; // unused
    const char *js = "[1 000.0]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_number_1.0e+.json
static void n_number_1dot0_eplus(void **state)
{
    (void)state; // unused
    const char *js = "[1.0e+]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_1.0e-.json
static void n_number_1dot0_eminus(void **state)
{
    (void)state; // unused
    const char *js = "[1.0e-]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_1.0e.json
static void n_number_1dot0_e(void **state)
{
    (void)state; // unused
    const char *js = "[1.0e]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_-1.0..json
static void n_number_minus1dot0dot(void **state)
{
    (void)state; // unused
    const char *js = "[-1.0.]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_1eE2.json
static void n_number_1_eE2(void **state)
{
    (void)state; // unused
    const char *js = "[1eE2]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_+1.json
static void n_number_plus1(void **state)
{
    (void)state; // unused
    const char *js = "[+1]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_.-1.json
static void n_number_dotminus1(void **state)
{
    (void)state; // unused
    const char *js = "[.-1]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_2.e+3.json
static void n_number_2dot_eplus3(void **state)
{
    (void)state; // unused
    const char *js = "[2.e+3]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_2.e-3.json
static void n_number_2dot_eminus3(void **state)
{
    (void)state; // unused
    const char *js = "[2.e-3]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_2.e3.json
static void n_number_2dot_e3(void **state)
{
    (void)state; // unused
    const char *js = "[2.e3]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_.2e-3.json
static void n_number_dot2_eminus3(void **state)
{
    (void)state; // unused
    const char *js = "[.2e-3]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_-2..json
static void n_number_minus2dot(void **state)
{
    (void)state; // unused
    const char *js = "[-2.]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_9.e+.json
static void n_number_9dot_eplus(void **state)
{
    (void)state; // unused
    const char *js = "[9.e+]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_expression.json
static void n_number_expression(void **state)
{
    (void)state; // unused
    const char *js = "[1+2]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_hex_1_digit.json
static void n_number_hex_1_digit(void **state)
{
    (void)state; // unused
    const char *js = "[0x1]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_hex_2_digits.json
static void n_number_hex_2_digits(void **state)
{
    (void)state; // unused
    const char *js = "[0x42]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_infinity.json
static void n_number_infinity(void **state)
{
    (void)state; // unused
    const char *js = "[Infinity]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_+Inf.json
static void n_number_plusInf(void **state)
{
    (void)state; // unused
    const char *js = "[+Inf]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_Inf.json
static void n_number_Inf(void **state)
{
    (void)state; // unused
    const char *js = "[Inf]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_invalid+-.json
static void n_number_invalid_plusminus(void **state)
{
    (void)state; // unused
    const char *js = "[0e+-1]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_invalid-negative-real.json
static void n_number_invalid_negative_real(void **state)
{
    (void)state; // unused
    const char *js = "[-123.123foo]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_invalid-utf-8-in-bigger-int.json
static void n_number_invalid_utf8_in_bigger_int(void **state)
{
    (void)state; // unused
    const char *js = "[123Â]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_invalid-utf-8-in-exponent.json
static void n_number_invalid_utf8_in_exponent(void **state)
{
    (void)state; // unused
    const char *js = "[1e1Â]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_invalid-utf-8-in-int.json
static void n_number_invalid_utf8_in_int(void **state)
{
    (void)state; // unused
    const char *js = "[0Â]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_++.json
static void n_number_plusplus(void **state)
{
    (void)state; // unused
    const char *js = "[++1234]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_minus_infinity.json
static void n_number_minus_infinity(void **state)
{
    (void)state; // unused
    const char *js = "[-Infinity]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_minus_sign_with_trailing_garbage.json
static void n_number_minus_sign_with_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "[-foo]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_minus_space_1.json
static void n_number_minus_space_1(void **state)
{
    (void)state; // unused
    const char *js = "[- 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_number_-NaN.json
static void n_number_minusNaN(void **state)
{
    (void)state; // unused
    const char *js = "[-NaN]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_NaN.json
static void n_number_NaN(void **state)
{
    (void)state; // unused
    const char *js = "[NaN]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_neg_int_starting_with_zero.json
static void n_number_neg_int_starting_with_zero(void **state)
{
    (void)state; // unused
    const char *js = "[-012]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_neg_real_without_int_part.json
static void n_number_neg_real_without_int_part(void **state)
{
    (void)state; // unused
    const char *js = "[-.123]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_neg_with_garbage_at_end.json
static void n_number_neg_with_garbage_at_end(void **state)
{
    (void)state; // unused
    const char *js = "[-1x]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_real_garbage_after_e.json
static void n_number_real_garbage_after_e(void **state)
{
    (void)state; // unused
    const char *js = "[1ea]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_real_with_invalid_utf8_after_e.json
static void n_number_real_with_invalid_utf8_after_e(void **state)
{
    (void)state; // unused
    const char *js = "[1eÂ]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_real_without_fractional_part.json
static void n_number_real_without_fractional_part(void **state)
{
    (void)state; // unused
    const char *js = "[1.]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_starting_with_dot.json
static void n_number_starting_with_dot(void **state)
{
    (void)state; // unused
    const char *js = "[.123]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_U+FF11_fullwidth_digit_one.json
static void n_number_U_FF11_fullwidth_digit_one(void **state)
{
    (void)state; // unused
    const char *js = "[Ôºë]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_with_alpha_char.json
static void n_number_with_alpha_char(void **state)
{
    (void)state; // unused
    const char *js = "[1.8011670033376514H-308]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_with_alpha.json
static void n_number_with_alpha(void **state)
{
    (void)state; // unused
    const char *js = "[1.2a-3]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_with_leading_zero.json
static void n_number_with_leading_zero(void **state)
{
    (void)state; // unused
    const char *js = "[012]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_number_zero_zero.json
static void n_number_zero_zero(void **state)
{
    (void)state; // unused
    const char *js = "00";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_object_bad_value.json
static void n_object_bad_value(void **state)
{
    (void)state; // unused
    const char *js = "[\"x\", truth]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_bracket_key.json
static void n_object_bracket_key(void **state)
{
    (void)state; // unused
    const char *js = "{[: \"x\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_comma_instead_of_colon.json
static void n_object_comma_instead_of_colon(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\", null}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_double_colon.json
static void n_object_double_colon(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\"::\"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_emoji.json
static void n_object_emoji(void **state)
{
    (void)state; // unused
    const char *js = "{üá®üá≠}";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_object_garbage_at_end.json
static void n_object_garbage_at_end(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"a\" 123}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_key_with_single_quotes.json
static void n_object_key_with_single_quotes(void **state)
{
    (void)state; // unused
    const char *js = "{key: 'value'}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_lone_continuation_byte_in_key_and_trailing_comma.json
static void n_object_lone_continuation_byte_in_key_and_trailing_comma(void **state)
{
    (void)state; // unused
    const char *js = "{\"π\":\"0\",}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_missing_colon.json
static void n_object_missing_colon(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\" b}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_missing_key.json
static void n_object_missing_key(void **state)
{
    (void)state; // unused
    const char *js = "{:\"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_missing_semicolon.json
static void n_object_missing_semicolon(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\" \"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_missing_value.json
static void n_object_missing_value(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_object_no-colon.json
static void n_object_no_colon(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_object_non_string_key_but_huge_number_instead.json
static void n_object_non_string_key_but_huge_number_instead(void **state)
{
    (void)state; // unused
    const char *js = "{9999E9999:1}";
#ifndef JSMN_PERMISSIVE_KEY
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_non_string_key.json
static void n_object_non_string_key(void **state)
{
    (void)state; // unused
    const char *js = "{1:1}";
#ifndef JSMN_PERMISSIVE_KEY
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_repeated_null_null.json
static void n_object_repeated_null_null(void **state)
{
    (void)state; // unused
    const char *js = "{null:null,null:null}";
#ifndef JSMN_PERMISSIVE_KEY
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 5);
#endif
}

// n_object_several_trailing_commas.json
static void n_object_several_trailing_commas(void **state)
{
    (void)state; // unused
    const char *js = "{\"id\":0,,,,,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_single_quote.json
static void n_object_single_quote(void **state)
{
    (void)state; // unused
    const char *js = "{'a':0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_trailing_comma.json
static void n_object_trailing_comma(void **state)
{
    (void)state; // unused
    const char *js = "{\"id\":0,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_trailing_comment.json
static void n_object_trailing_comment(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}/**/";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_trailing_comment_open.json
static void n_object_trailing_comment_open(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}/**//";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_trailing_comment_slash_open_incomplete.json
static void n_object_trailing_comment_slash_open_incomplete(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}/";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_trailing_comment_slash_open.json
static void n_object_trailing_comment_slash_open(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}//";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_two_commas_in_a_row.json
static void n_object_two_commas_in_a_row(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\",,\"c\":\"d\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_unquoted_key.json
static void n_object_unquoted_key(void **state)
{
    (void)state; // unused
    const char *js = "{a: \"b\"}";
#if !defined(JSMN_PERMISSIVE_PRIMITIVE) && !defined(JSMN_PERMISSIVE_KEY)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_object_unterminated-value.json
static void n_object_unterminated_value(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"a";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_object_with_single_string.json
static void n_object_with_single_string(void **state)
{
    (void)state; // unused
    const char *js = "{ \"foo\" : \"bar\", \"a\" }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_object_with_trailing_garbage.json
static void n_object_with_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}#";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_single_space.json
static void n_single_space(void **state)
{
    (void)state; // unused
    const char *js = " ";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_1_surrogate_then_escape.json
static void n_string_1_surrogate_then_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_1_surrogate_then_escape_u1.json
static void n_string_1_surrogate_then_escape_u1(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\u1\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_1_surrogate_then_escape_u1x.json
static void n_string_1_surrogate_then_escape_u1x(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\u1x\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_1_surrogate_then_escape_u.json
static void n_string_1_surrogate_then_escape_u(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\u\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_accentuated_char_no_quotes.json
static void n_string_accentuated_char_no_quotes(void **state)
{
    (void)state; // unused
    const char *js = "[√©]";
#ifndef JSMN_PERMISSIVE_PRIMITIVE	
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_string_backslash_00.json
static void n_string_backslash_00(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\0\"]";
    assert_int_equal(jsmn_parse(&p, js, 6, NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_escaped_backslash_bad.json
static void n_string_escaped_backslash_bad(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\\\\\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_escaped_ctrl_char_tab.json
static void n_string_escaped_ctrl_char_tab(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\t\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_escaped_emoji.json
static void n_string_escaped_emoji(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\üåÄ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_escape_x.json
static void n_string_escape_x(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\x00\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_incomplete_escaped_character.json
static void n_string_incomplete_escaped_character(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u00A\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_incomplete_escape.json
static void n_string_incomplete_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_incomplete_surrogate_escape_invalid.json
static void n_string_incomplete_surrogate_escape_invalid(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD800\\uD800\\x\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_incomplete_surrogate.json
static void n_string_incomplete_surrogate(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD834\\uDd\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_invalid_backslash_esc.json
static void n_string_invalid_backslash_esc(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\a\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_invalid_unicode_escape.json
static void n_string_invalid_unicode_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uqqqq\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_invalid_utf8_after_escape.json
static void n_string_invalid_utf8_after_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\Â\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_invalid-utf-8-in-escape.json
static void n_string_invalid_utf8_in_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uÂ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_leading_uescaped_thinspace.json
static void n_string_leading_uescaped_thinspace(void **state)
{
    (void)state; // unused
    const char *js = "[\\u0020\"asd\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_no_quotes_with_bad_escape.json
static void n_string_no_quotes_with_bad_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\\n]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_single_doublequote.json
static void n_string_single_doublequote(void **state)
{
    (void)state; // unused
    const char *js = "\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_single_quote.json
static void n_string_single_quote(void **state)
{
    (void)state; // unused
    const char *js = "['single quote']";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_single_string_no_double_quotes.json
static void n_string_single_string_no_double_quotes(void **state)
{
    (void)state; // unused
    const char *js = "abc";
#ifndef JSMN_PERMISSIVE_PRIMITIVE
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_string_start_escape_unclosed.json
static void n_string_start_escape_unclosed(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_unescaped_crtl_char.json
static void n_string_unescaped_crtl_char(void **state)
{
    (void)state; // unused
    const char *js = "[\"aa\0\"]";
    assert_int_equal(jsmn_parse(&p, js, 7, NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_string_unescaped_newline.json
static void n_string_unescaped_newline(void **state)
{
    (void)state; // unused
    const char *js = "[\"new\nline\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_unescaped_tab.json
static void n_string_unescaped_tab(void **state)
{
    (void)state; // unused
    const char *js = "[\"\t\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_unicode_CapitalU.json
static void n_string_unicode_CapitalU(void **state)
{
    (void)state; // unused
    const char *js = "\"\\UA66D\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_string_with_trailing_garbage.json
static void n_string_with_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "\"\"x";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_structure_angle_bracket_..json
static void n_structure_angle_bracket_dot(void **state)
{
    (void)state; // unused
    const char *js = "<.>";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_angle_bracket_null.json
static void n_structure_angle_bracket_null(void **state)
{
    (void)state; // unused
    const char *js = "[<null>]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_array_trailing_garbage.json
static void n_structure_array_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "[1]x";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_structure_array_with_extra_array_close.json
static void n_structure_array_with_extra_array_close(void **state)
{
    (void)state; // unused
    const char *js = "[1]]";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#endif
}

// n_structure_array_with_unclosed_string.json
static void n_structure_array_with_unclosed_string(void **state)
{
    (void)state; // unused
    const char *js = "[\"asd]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_ascii-unicode-identifier.json
static void n_structure_ascii_unicode_identifier(void **state)
{
    (void)state; // unused
    const char *js = "a√•";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_capitalized_True.json
static void n_structure_capitalized_True(void **state)
{
    (void)state; // unused
    const char *js = "[True]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_close_unopened_array.json
static void n_structure_close_unopened_array(void **state)
{
    (void)state; // unused
    const char *js = "1]";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_structure_comma_instead_of_closing_brace.json
static void n_structure_comma_instead_of_closing_brace(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\": true,";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_double_array.json
static void n_structure_double_array(void **state)
{
    (void)state; // unused
    const char *js = "[][]";
#if defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#elif defined(JSMN_MULTIPLE_JSON)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_structure_end_array.json
static void n_structure_end_array(void **state)
{
    (void)state; // unused
    const char *js = "]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_incomplete_UTF8_BOM.json
static void n_structure_incomplete_UTF8_BOM(void **state)
{
    (void)state; // unused
    const char *js = "Ôª{}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_lone-invalid-utf-8.json
static void n_structure_lone_invalid_utf8(void **state)
{
    (void)state; // unused
    const char *js = "Â";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_lone-open-bracket.json
static void n_structure_lone_open_bracket(void **state)
{
    (void)state; // unused
    const char *js = "[";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_no_data.json
static void n_structure_no_data(void **state)
{
    (void)state; // unused
    const char *js = "";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_null-byte-outside-string.json
static void n_structure_null_byte_outside_string(void **state)
{
    (void)state; // unused
    const char *js = "[\0]";
    assert_int_equal(jsmn_parse(&p, js, 3, NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_number_with_trailing_garbage.json
static void n_structure_number_with_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "2@";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_object_followed_by_closing_object.json
static void n_structure_object_followed_by_closing_object(void **state)
{
    (void)state; // unused
    const char *js = "{}}";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
#endif
}

// n_structure_object_unclosed_no_value.json
static void n_structure_object_unclosed_no_value(void **state)
{
    (void)state; // unused
    const char *js = "{\"\":";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_object_with_comment.json
static void n_structure_object_with_comment(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":/*comment*/\"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_object_with_trailing_garbage.json
static void n_structure_object_with_trailing_garbage(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": true} \"x\"";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_structure_open_array_apostrophe.json
static void n_structure_open_array_apostrophe(void **state)
{
    (void)state; // unused
    const char *js = "['";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_open_array_comma.json
static void n_structure_open_array_comma(void **state)
{
    (void)state; // unused
    const char *js = "[,";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_open_array_object.json is too big.

// n_structure_open_array_open_object.json
static void n_structure_open_array_open_object(void **state)
{
    (void)state; // unused
    const char *js = "[{";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_open_array_open_string.json
static void n_structure_open_array_open_string(void **state)
{
    (void)state; // unused
    const char *js = "[\"a";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_open_array_string.json
static void n_structure_open_array_string(void **state)
{
    (void)state; // unused
    const char *js = "[\"a\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_open_object_close_array.json
static void n_structure_open_object_close_array(void **state)
{
    (void)state; // unused
    const char *js = "{]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

// n_structure_open_object_comma.json
static void n_structure_open_object_comma(void **state)
{
    (void)state; // unused
    const char *js = "{,";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_open_object.json
static void n_structure_open_object(void **state)
{
    (void)state; // unused
    const char *js = "{";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_open_object_open_array.json
static void n_structure_open_object_open_array(void **state)
{
    (void)state; // unused
    const char *js = "{[";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_open_object_open_string.json
static void n_structure_open_object_open_string(void **state)
{
    (void)state; // unused
    const char *js = "{\"a";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_open_object_string_with_apostrophes.json
static void n_structure_open_object_string_with_apostrophes(void **state)
{
    (void)state; // unused
    const char *js = "{'a'";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_open_open.json
static void n_structure_open_open(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\{[\"\\{[\"\\{[\"\\{";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_single_eacute.json
static void n_structure_single_eacute(void **state)
{
    (void)state; // unused
    const char *js = "È";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_single_star.json
static void n_structure_single_star(void **state)
{
    (void)state; // unused
    const char *js = "*";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_trailing_#.json
static void n_structure_trailing_sharp(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\"}#{}";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
#endif
}

// n_structure_U+2060_word_joined.json
static void n_structure_U_2060_word_joined(void **state)
{
    (void)state; // unused
    const char *js = "[‚Å†]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_uescaped_LF_before_string.json
static void n_structure_uescaped_LF_before_string(void **state)
{
    (void)state; // unused
    const char *js = "[\\u000A\"\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_unclosed_array.json
static void n_structure_unclosed_array(void **state)
{
    (void)state; // unused
    const char *js = "[1";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_unclosed_array_partial_null.json
static void n_structure_unclosed_array_partial_null(void **state)
{
    (void)state; // unused
    const char *js = "[ false, nul";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_unclosed_array_unfinished_false.json
static void n_structure_unclosed_array_unfinished_false(void **state)
{
    (void)state; // unused
    const char *js = "[ true, fals";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_unclosed_array_unfinished_true.json
static void n_structure_unclosed_array_unfinished_true(void **state)
{
    (void)state; // unused
    const char *js = "[ false, tru";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_unclosed_object.json
static void n_structure_unclosed_object(void **state)
{
    (void)state; // unused
    const char *js = "{\"asd\":\"asd\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
}

// n_structure_unicode-identifier.json
static void n_structure_unicode_identifier(void **state)
{
    (void)state; // unused
    const char *js = "√•";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_UTF8_BOM_no_data.json
static void n_structure_UTF8_BOM_no_data(void **state)
{
    (void)state; // unused
    const char *js = "Ôªø";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_whitespace_formfeed.json
static void n_structure_whitespace_formfeed(void **state)
{
    (void)state; // unused
    const char *js = "[\f]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

// n_structure_whitespace_U+2060_word_joiner.json
static void n_structure_whitespace_U_2060_word_joiner(void **state)
{
    (void)state; // unused
    const char *js = "[‚Å†]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_INVAL);
}

void test_jsontestsuite_n(void)
{
    const struct CMUnitTest tests[] = {
#ifndef JSMN_PERMISSIVE
        cmocka_unit_test_setup(n_array_1_true_without_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_a_invalid_utf8, jsmn_setup),
        cmocka_unit_test_setup(n_array_colon_instead_of_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_comma_after_close, jsmn_setup),
        cmocka_unit_test_setup(n_array_comma_and_number, jsmn_setup),
        cmocka_unit_test_setup(n_array_double_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_double_extra_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_extra_close, jsmn_setup),
        cmocka_unit_test_setup(n_array_extra_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_incomplete, jsmn_setup),
        cmocka_unit_test_setup(n_array_incomplete_invalid_value, jsmn_setup),
        cmocka_unit_test_setup(n_array_inner_array_no_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_invalid_utf8, jsmn_setup),
        cmocka_unit_test_setup(n_array_items_separated_by_semicolon, jsmn_setup),
        cmocka_unit_test_setup(n_array_just_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_just_minus, jsmn_setup),
        cmocka_unit_test_setup(n_array_missing_value, jsmn_setup),
        cmocka_unit_test_setup(n_array_newlines_unclosed, jsmn_setup),
        cmocka_unit_test_setup(n_array_number_and_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_number_and_several_commas, jsmn_setup),
        cmocka_unit_test_setup(n_array_spaces_vertical_tab_formfeed, jsmn_setup),
        cmocka_unit_test_setup(n_array_star_inside, jsmn_setup),
        cmocka_unit_test_setup(n_array_unclosed, jsmn_setup),
        cmocka_unit_test_setup(n_array_unclosed_trailing_comma, jsmn_setup),
        cmocka_unit_test_setup(n_array_unclosed_with_new_lines, jsmn_setup),
        cmocka_unit_test_setup(n_array_unclosed_with_object_inside, jsmn_setup),
        cmocka_unit_test_setup(n_incomplete_false, jsmn_setup),
        cmocka_unit_test_setup(n_incomplete_null, jsmn_setup),
        cmocka_unit_test_setup(n_incomplete_true, jsmn_setup),
        cmocka_unit_test_setup(n_multidigit_number_then_00, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus01, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus1dot0dot, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus2dot, jsmn_setup),
        cmocka_unit_test_setup(n_number_minusNaN, jsmn_setup),
        cmocka_unit_test_setup(n_number_dotminus1, jsmn_setup),
        cmocka_unit_test_setup(n_number_dot2_eminus3, jsmn_setup),
        cmocka_unit_test_setup(n_number_plusplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_plus1, jsmn_setup),
        cmocka_unit_test_setup(n_number_plusInf, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_0dot1dot2, jsmn_setup),
        cmocka_unit_test_setup(n_number_0dot3_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_capital_Eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_capital_E, jsmn_setup),
        cmocka_unit_test_setup(n_number_0dot3_eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_0dot_e1, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_0_eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_1_000, jsmn_setup),
        cmocka_unit_test_setup(n_number_1dot0_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_1dot0_eminus, jsmn_setup),
        cmocka_unit_test_setup(n_number_1dot0_eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_1_eE2, jsmn_setup),
        cmocka_unit_test_setup(n_number_2dot_eminus3, jsmn_setup),
        cmocka_unit_test_setup(n_number_2dot_eplus3, jsmn_setup),
        cmocka_unit_test_setup(n_number_2dot_e3, jsmn_setup),
        cmocka_unit_test_setup(n_number_9dot_eplus, jsmn_setup),
        cmocka_unit_test_setup(n_number_expression, jsmn_setup),
        cmocka_unit_test_setup(n_number_hex_1_digit, jsmn_setup),
        cmocka_unit_test_setup(n_number_hex_2_digits, jsmn_setup),
        cmocka_unit_test_setup(n_number_Inf, jsmn_setup),
        cmocka_unit_test_setup(n_number_infinity, jsmn_setup),
        cmocka_unit_test_setup(n_number_invalid_negative_real, jsmn_setup),
        cmocka_unit_test_setup(n_number_invalid_utf8_in_bigger_int, jsmn_setup),
        cmocka_unit_test_setup(n_number_invalid_utf8_in_exponent, jsmn_setup),
        cmocka_unit_test_setup(n_number_invalid_utf8_in_int, jsmn_setup),
        cmocka_unit_test_setup(n_number_invalid_plusminus, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus_infinity, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus_sign_with_trailing_garbage, jsmn_setup),
        cmocka_unit_test_setup(n_number_minus_space_1, jsmn_setup),
        cmocka_unit_test_setup(n_number_NaN, jsmn_setup),
        cmocka_unit_test_setup(n_number_neg_int_starting_with_zero, jsmn_setup),
        cmocka_unit_test_setup(n_number_neg_real_without_int_part, jsmn_setup),
        cmocka_unit_test_setup(n_number_neg_with_garbage_at_end, jsmn_setup),
        cmocka_unit_test_setup(n_number_real_garbage_after_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_real_with_invalid_utf8_after_e, jsmn_setup),
        cmocka_unit_test_setup(n_number_real_without_fractional_part, jsmn_setup),
        cmocka_unit_test_setup(n_number_starting_with_dot, jsmn_setup),
        cmocka_unit_test_setup(n_number_U_FF11_fullwidth_digit_one, jsmn_setup),
        cmocka_unit_test_setup(n_number_with_alpha, jsmn_setup),
        cmocka_unit_test_setup(n_number_with_alpha_char, jsmn_setup),
        cmocka_unit_test_setup(n_number_with_leading_zero, jsmn_setup),
        cmocka_unit_test_setup(n_object_bad_value, jsmn_setup),
        cmocka_unit_test_setup(n_object_bracket_key, jsmn_setup),
        cmocka_unit_test_setup(n_object_comma_instead_of_colon, jsmn_setup),
        cmocka_unit_test_setup(n_object_double_colon, jsmn_setup),
        cmocka_unit_test_setup(n_object_emoji, jsmn_setup),
        cmocka_unit_test_setup(n_object_garbage_at_end, jsmn_setup),
        cmocka_unit_test_setup(n_object_key_with_single_quotes, jsmn_setup),
        cmocka_unit_test_setup(n_object_lone_continuation_byte_in_key_and_trailing_comma, jsmn_setup),
        cmocka_unit_test_setup(n_object_missing_colon, jsmn_setup),
        cmocka_unit_test_setup(n_object_missing_key, jsmn_setup),
        cmocka_unit_test_setup(n_object_missing_semicolon, jsmn_setup),
        cmocka_unit_test_setup(n_object_missing_value, jsmn_setup),
        cmocka_unit_test_setup(n_object_no_colon, jsmn_setup),
        cmocka_unit_test_setup(n_object_non_string_key, jsmn_setup),
        cmocka_unit_test_setup(n_object_non_string_key_but_huge_number_instead, jsmn_setup),
        cmocka_unit_test_setup(n_object_repeated_null_null, jsmn_setup),
        cmocka_unit_test_setup(n_object_several_trailing_commas, jsmn_setup),
        cmocka_unit_test_setup(n_object_single_quote, jsmn_setup),
        cmocka_unit_test_setup(n_object_trailing_comma, jsmn_setup),
        cmocka_unit_test_setup(n_object_trailing_comment, jsmn_setup),
        cmocka_unit_test_setup(n_object_trailing_comment_open, jsmn_setup),
        cmocka_unit_test_setup(n_object_trailing_comment_slash_open, jsmn_setup),
        cmocka_unit_test_setup(n_object_trailing_comment_slash_open_incomplete, jsmn_setup),
        cmocka_unit_test_setup(n_object_two_commas_in_a_row, jsmn_setup),
        cmocka_unit_test_setup(n_object_unquoted_key, jsmn_setup),
        cmocka_unit_test_setup(n_object_unterminated_value, jsmn_setup),
        cmocka_unit_test_setup(n_object_with_single_string, jsmn_setup),
        cmocka_unit_test_setup(n_object_with_trailing_garbage, jsmn_setup),
        cmocka_unit_test_setup(n_single_space, jsmn_setup),
        cmocka_unit_test_setup(n_string_1_surrogate_then_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_1_surrogate_then_escape_u, jsmn_setup),
        cmocka_unit_test_setup(n_string_1_surrogate_then_escape_u1, jsmn_setup),
        cmocka_unit_test_setup(n_string_1_surrogate_then_escape_u1x, jsmn_setup),
        cmocka_unit_test_setup(n_string_accentuated_char_no_quotes, jsmn_setup),
        cmocka_unit_test_setup(n_string_backslash_00, jsmn_setup),
        cmocka_unit_test_setup(n_string_escape_x, jsmn_setup),
        cmocka_unit_test_setup(n_string_escaped_backslash_bad, jsmn_setup),
        cmocka_unit_test_setup(n_string_escaped_ctrl_char_tab, jsmn_setup),
        cmocka_unit_test_setup(n_string_escaped_emoji, jsmn_setup),
        cmocka_unit_test_setup(n_string_incomplete_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_incomplete_escaped_character, jsmn_setup),
        cmocka_unit_test_setup(n_string_incomplete_surrogate, jsmn_setup),
        cmocka_unit_test_setup(n_string_incomplete_surrogate_escape_invalid, jsmn_setup),
        cmocka_unit_test_setup(n_string_invalid_backslash_esc, jsmn_setup),
        cmocka_unit_test_setup(n_string_invalid_unicode_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_invalid_utf8_after_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_invalid_utf8_in_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_leading_uescaped_thinspace, jsmn_setup),
        cmocka_unit_test_setup(n_string_no_quotes_with_bad_escape, jsmn_setup),
        cmocka_unit_test_setup(n_string_single_doublequote, jsmn_setup),
        cmocka_unit_test_setup(n_string_single_quote, jsmn_setup),
        cmocka_unit_test_setup(n_string_single_string_no_double_quotes, jsmn_setup),
        cmocka_unit_test_setup(n_string_start_escape_unclosed, jsmn_setup),
        cmocka_unit_test_setup(n_string_unescaped_crtl_char, jsmn_setup),
        cmocka_unit_test_setup(n_string_unescaped_newline, jsmn_setup),
        cmocka_unit_test_setup(n_string_unescaped_tab, jsmn_setup),
        cmocka_unit_test_setup(n_string_unicode_CapitalU, jsmn_setup),
        cmocka_unit_test_setup(n_string_with_trailing_garbage, jsmn_setup),
#ifndef JSMN_SHORT_TOKENS
//      cmocka_unit_test_setup(n_structure_100000_opening_arrays, jsmn_setup),
#endif
        cmocka_unit_test_setup(n_structure_angle_bracket_dot, jsmn_setup),
        cmocka_unit_test_setup(n_structure_angle_bracket_null, jsmn_setup),
        cmocka_unit_test_setup(n_structure_array_trailing_garbage, jsmn_setup),
        cmocka_unit_test_setup(n_structure_array_with_extra_array_close, jsmn_setup),
        cmocka_unit_test_setup(n_structure_array_with_unclosed_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_ascii_unicode_identifier, jsmn_setup),
        cmocka_unit_test_setup(n_structure_capitalized_True, jsmn_setup),
        cmocka_unit_test_setup(n_structure_close_unopened_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_comma_instead_of_closing_brace, jsmn_setup),
        cmocka_unit_test_setup(n_structure_double_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_end_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_incomplete_UTF8_BOM, jsmn_setup),
        cmocka_unit_test_setup(n_structure_lone_invalid_utf8, jsmn_setup),
        cmocka_unit_test_setup(n_structure_lone_open_bracket, jsmn_setup),
        cmocka_unit_test_setup(n_structure_no_data, jsmn_setup),
        cmocka_unit_test_setup(n_structure_null_byte_outside_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_number_with_trailing_garbage, jsmn_setup),
        cmocka_unit_test_setup(n_structure_object_followed_by_closing_object, jsmn_setup),
        cmocka_unit_test_setup(n_structure_object_unclosed_no_value, jsmn_setup),
        cmocka_unit_test_setup(n_structure_object_with_comment, jsmn_setup),
        cmocka_unit_test_setup(n_structure_object_with_trailing_garbage, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_array_apostrophe, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_array_comma, jsmn_setup),
//      cmocka_unit_test_setup(n_structure_open_array_object, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_array_open_object, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_array_open_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_array_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object_close_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object_comma, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object_open_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object_open_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_object_string_with_apostrophes, jsmn_setup),
        cmocka_unit_test_setup(n_structure_open_open, jsmn_setup),
        cmocka_unit_test_setup(n_structure_single_eacute, jsmn_setup),
        cmocka_unit_test_setup(n_structure_single_star, jsmn_setup),
        cmocka_unit_test_setup(n_structure_trailing_sharp, jsmn_setup),
        cmocka_unit_test_setup(n_structure_U_2060_word_joined, jsmn_setup),
        cmocka_unit_test_setup(n_structure_uescaped_LF_before_string, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unclosed_array, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unclosed_array_partial_null, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unclosed_array_unfinished_false, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unclosed_array_unfinished_true, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unclosed_object, jsmn_setup),
        cmocka_unit_test_setup(n_structure_unicode_identifier, jsmn_setup),
        cmocka_unit_test_setup(n_structure_UTF8_BOM_no_data, jsmn_setup),
        cmocka_unit_test_setup(n_structure_whitespace_formfeed, jsmn_setup),
        cmocka_unit_test_setup(n_structure_whitespace_U_2060_word_joiner, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("JSONTestSuite tests that shouldn't pass.", tests, NULL, NULL);
}

// y_array_arraysWithSpaces.json
static void y_array_arraysWithSpaces(void **state)
{
    (void)state; // unused
    const char *js = "[[]   ]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_ARRAY,  1, 3, 0);
}

// y_array_empty.json
static void y_array_empty(void **state)
{
    (void)state; // unused
    const char *js = "[]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_ARRAY,  0, 2, 0);
}

// y_array_empty-string.json
static void y_array_empty_string(void **state)
{
    (void)state; // unused
    const char *js = "[\"\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_STRING, "", 0);
}

// y_array_false.json
static void y_array_false(void **state)
{
    (void)state; // unused
    const char *js = "[false]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_PRIMITIVE, "false");
}

// y_array_heterogeneous.json
static void y_array_heterogeneous(void **state)
{
    (void)state; // unused
    const char *js = "[null, 1, \"1\", {}]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_ARRAY,  0, 18, 4,
          JSMN_PRIMITIVE, "null",
          JSMN_PRIMITIVE, "1",
          JSMN_STRING, "1", 0,
          JSMN_OBJECT, 15, 17, 0);
}

// y_array_null.json
static void y_array_null(void **state)
{
    (void)state; // unused
    const char *js = "[null]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "null");
}

// y_array_with_1_and_newline.json
static void y_array_with_1_and_newline(void **state)
{
    (void)state; // unused
    const char *js = "[1\n]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_PRIMITIVE, "1");
}

// y_array_with_leading_space.json
static void y_array_with_leading_space(void **state)
{
    (void)state; // unused
    const char *js = " [1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  1, 4, 1,
          JSMN_PRIMITIVE, "1");
}

// y_array_with_several_null.json
static void y_array_with_several_null(void **state)
{
    (void)state; // unused
    const char *js = "[1,null,null,null,2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), 6);
    tokeq(js, t, 6,
          JSMN_ARRAY,  0, 20, 5,
          JSMN_PRIMITIVE, "1",
          JSMN_PRIMITIVE, "null",
          JSMN_PRIMITIVE, "null",
          JSMN_PRIMITIVE, "null",
          JSMN_PRIMITIVE, "2");
}

// y_array_with_trailing_space.json
static void y_array_with_trailing_space(void **state)
{
    (void)state; // unused
    const char *js = "[2] ";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 3, 1,
          JSMN_PRIMITIVE, "2");
}

// y_number_0e+1.json
static void y_number_0_eplus1(void **state)
{
    (void)state; // unused
    const char *js = "[0e+1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "0e+1");
}

// y_number_0e1.json
static void y_number_0e1(void **state)
{
    (void)state; // unused
    const char *js = "[0e1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_PRIMITIVE, "0e1");
}

// y_number_after_space.json
static void y_number_after_space(void **state)
{
    (void)state; // unused
    const char *js = "[ 4]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_PRIMITIVE, "4");
}

// y_number_double_close_to_zero.json
static void y_number_double_close_to_zero(void **state)
{
    (void)state; // unused
    const char *js = "[-0.000000000000000000000000000000000000000000000000000000000000000000000000000001]\
";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 83, 1,
          JSMN_PRIMITIVE, "-0.000000000000000000000000000000000000000000000000000000000000000000000000000001");
}

// y_number_int_with_exp.json
static void y_number_int_with_exp(void **state)
{
    (void)state; // unused
    const char *js = "[20e1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "20e1");
}

// y_number.json
static void y_number(void **state)
{
    (void)state; // unused
    const char *js = "[123e65]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_PRIMITIVE, "123e65");
}

// y_number_minus_zero.json
static void y_number_minus_zero(void **state)
{
    (void)state; // unused
    const char *js = "[-0]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_PRIMITIVE, "-0");
}

// y_number_negative_int.json
static void y_number_negative_int(void **state)
{
    (void)state; // unused
    const char *js = "[-123]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "-123");
}

// y_number_negative_one.json
static void y_number_negative_one(void **state)
{
    (void)state; // unused
    const char *js = "[-1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_PRIMITIVE, "-1");
}

// y_number_negative_zero.json
static void y_number_negative_zero(void **state)
{
    (void)state; // unused
    const char *js = "[-0]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 4, 1,
          JSMN_PRIMITIVE, "-0");
}

// y_number_real_capital_e.json
static void y_number_real_capital_e(void **state)
{
    (void)state; // unused
    const char *js = "[1E22]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "1E22");
}

// y_number_real_capital_e_neg_exp.json
static void y_number_real_capital_e_neg_exp(void **state)
{
    (void)state; // unused
    const char *js = "[1E-2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "1E-2");
}

// y_number_real_capital_e_pos_exp.json
static void y_number_real_capital_e_pos_exp(void **state)
{
    (void)state; // unused
    const char *js = "[1E+2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "1E+2");
}

// y_number_real_exponent.json
static void y_number_real_exponent(void **state)
{
    (void)state; // unused
    const char *js = "[123e45]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_PRIMITIVE, "123e45");
}

// y_number_real_fraction_exponent.json
static void y_number_real_fraction_exponent(void **state)
{
    (void)state; // unused
    const char *js = "[123.456e78]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 12, 1,
          JSMN_PRIMITIVE, "123.456e78");
}

// y_number_real_neg_exp.json
static void y_number_real_neg_exp(void **state)
{
    (void)state; // unused
    const char *js = "[1e-2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "1e-2");
}

// y_number_real_pos_exponent.json
static void y_number_real_pos_exponent(void **state)
{
    (void)state; // unused
    const char *js = "[1e+2]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "1e+2");
}

// y_number_simple_int.json
static void y_number_simple_int(void **state)
{
    (void)state; // unused
    const char *js = "[123]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_PRIMITIVE, "123");
}

// y_number_simple_real.json
static void y_number_simple_real(void **state)
{
    (void)state; // unused
    const char *js = "[123.456789]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 12, 1,
          JSMN_PRIMITIVE, "123.456789");
}

// y_object_basic.json
static void y_object_basic(void **state)
{
    (void)state; // unused
    const char *js = "{\"asd\":\"sdf\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 13, 1,
          JSMN_STRING, "asd", 1,
          JSMN_STRING, "sdf", 0);
}

// y_object_duplicated_key_and_value.json
static void y_object_duplicated_key_and_value(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\",\"a\":\"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, 0, 17, 2,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "b", 0,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "b", 0);
}

// y_object_duplicated_key.json
static void y_object_duplicated_key(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\",\"a\":\"c\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, 0, 17, 2,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "b", 0,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "c", 0);
}

// y_object_empty.json
static void y_object_empty(void **state)
{
    (void)state; // unused
    const char *js = "{}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_OBJECT, 0, 2, 0);
}

// y_object_empty_key.json
static void y_object_empty_key(void **state)
{
    (void)state; // unused
    const char *js = "{\"\":0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 6, 1,
          JSMN_STRING, "", 1,
          JSMN_PRIMITIVE, "0");
}

// y_object_escaped_null_in_key.json
static void y_object_escaped_null_in_key(void **state)
{
    (void)state; // unused
    const char *js = "{\"foo\\u0000bar\": 42}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 20, 1,
          JSMN_STRING, "foo\\u0000bar", 1,
          JSMN_PRIMITIVE, "42");
}

// y_object_extreme_numbers.json
static void y_object_extreme_numbers(void **state)
{
    (void)state; // unused
    const char *js = "{ \"min\": -1.0e+28, \"max\": 1.0e+28 }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, 0, 35, 2,
          JSMN_STRING, "min", 1,
          JSMN_PRIMITIVE, "-1.0e+28",
          JSMN_STRING, "max", 1,
          JSMN_PRIMITIVE, "1.0e+28");
}

// y_object.json
static void y_object(void **state)
{
    (void)state; // unused
    const char *js = "{\"asd\":\"sdf\", \"dfg\":\"fgh\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, 0, 26, 2,
          JSMN_STRING, "asd", 1,
          JSMN_STRING, "sdf", 0,
          JSMN_STRING, "dfg", 1,
          JSMN_STRING, "fgh", 0);
}

// y_object_long_strings.json
static void y_object_long_strings(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\":[{\"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}], \"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), 8);
    tokeq(js, t, 8,
          JSMN_OBJECT, 0, 108, 2,
          JSMN_STRING, "x", 1,
          JSMN_ARRAY,  5, 57, 1,
          JSMN_OBJECT, 6, 56, 1,
          JSMN_STRING, "id", 1,
          JSMN_STRING, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0,
          JSMN_STRING, "id", 1,
          JSMN_STRING, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0);
}

// y_object_simple.json
static void y_object_simple(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":[]}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 8, 1,
          JSMN_STRING, "a", 1,
          JSMN_ARRAY,  5, 7, 0);
}

// y_object_string_unicode.json
static void y_object_string_unicode(void **state)
{
    (void)state; // unused
    const char *js = "{\"title\":\"\\u041f\\u043e\\u043b\\u0442\\u043e\\u0440\\u0430 \\u0417\\u0435\\u043c\\u043b\\u0435\\u043a\\u043e\\u043f\\u0430\" }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 110, 1,
          JSMN_STRING, "title", 1,
          JSMN_STRING, "\\u041f\\u043e\\u043b\\u0442\\u043e\\u0440\\u0430 \\u0417\\u0435\\u043c\\u043b\\u0435\\u043a\\u043e\\u043f\\u0430", 0);
}

// y_object_with_newlines.json
static void y_object_with_newlines(void **state)
{
    (void)state; // unused
    const char *js = "{\n\"a\": \"b\"\n}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 12, 1,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "b", 0);
}

// y_string_1_2_3_bytes_UTF-8_sequences.json
static void y_string_1_2_3_bytes_UTF8_sequences(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0060\\u012a\\u12AB\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 22, 1,
          JSMN_STRING, "\\u0060\\u012a\\u12AB", 0);
}

// y_string_accepted_surrogate_pair.json
static void y_string_accepted_surrogate_pair(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD801\\udc37\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uD801\\udc37", 0);
}

// y_string_accepted_surrogate_pairs.json
static void y_string_accepted_surrogate_pairs(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\ud83d\\ude39\\ud83d\\udc8d\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 28, 1,
          JSMN_STRING, "\\ud83d\\ude39\\ud83d\\udc8d", 0);
}

// y_string_allowed_escapes.json
static void y_string_allowed_escapes(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 20, 1,
          JSMN_STRING, "\\\"\\\\\\/\\b\\f\\n\\r\\t", 0);
}

// y_string_backslash_and_u_escaped_zero.json
static void y_string_backslash_and_u_escaped_zero(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\\u0000\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 11, 1,
          JSMN_STRING, "\\\\u0000", 0);
}

// y_string_backslash_doublequotes.json
static void y_string_backslash_doublequotes(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\"\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_STRING, "\\\"", 0);
}

// y_string_comments.json
static void y_string_comments(void **state)
{
    (void)state; // unused
    const char *js = "[\"a/*b*/c/*d//e\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 17, 1,
          JSMN_STRING, "a/*b*/c/*d//e", 0);
}

// y_string_double_escape_a.json
static void y_string_double_escape_a(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\\a\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "\\\\a", 0);
}

// y_string_double_escape_n.json
static void y_string_double_escape_n(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\\\n\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "\\\\n", 0);
}

// y_string_escaped_control_character.json
static void y_string_escaped_control_character(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0012\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u0012", 0);
}

// y_string_escaped_noncharacter.json
static void y_string_escaped_noncharacter(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uFFFF\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uFFFF", 0);
}

// y_string_in_array.json
static void y_string_in_array(void **state)
{
    (void)state; // unused
    const char *js = "[\"asd\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "asd", 0);
}

// y_string_in_array_with_leading_space.json
static void y_string_in_array_with_leading_space(void **state)
{
    (void)state; // unused
    const char *js = "[ \"asd\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_STRING, "asd", 0);
}

// y_string_last_surrogates_1_and_2.json
static void y_string_last_surrogates_1_and_2(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDBFF\\uDFFF\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uDBFF\\uDFFF", 0);
}

// y_string_nbsp_uescaped.json
static void y_string_nbsp_uescaped(void **state)
{
    (void)state; // unused
    const char *js = "[\"new\\u00A0line\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 17, 1,
          JSMN_STRING, "new\\u00A0line", 0);
}

// y_string_nonCharacterInUTF-8_U+10FFFF.json
static void y_string_nonCharacterInUTF8_U_10FFFF(void **state)
{
    (void)state; // unused
    const char *js = "[\"Ùèøø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_STRING, "Ùèøø", 0);
}

// y_string_nonCharacterInUTF-8_U+FFFF.json
static void y_string_nonCharacterInUTF8_U_FFFF(void **state)
{
    (void)state; // unused
    const char *js = "[\"Ôøø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "Ôøø", 0);
}

// y_string_null_escape.json
static void y_string_null_escape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0000\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u0000", 0);
}

// y_string_one-byte-utf-8.json
static void y_string_one_byte_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u002c\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u002c", 0);
}

// y_string_pi.json
static void y_string_pi(void **state)
{
    (void)state; // unused
    const char *js = "[\"œÄ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_STRING, "œÄ", 0);
}

// y_string_reservedCharacterInUTF-8_U+1BFFF.json
static void y_string_reservedCharacterInUTF8_U_1BFFF(void **state)
{
    (void)state; // unused
    const char *js = "[\"õøø\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_STRING, "õøø", 0);
}

// y_string_simple_ascii.json
static void y_string_simple_ascii(void **state)
{
    (void)state; // unused
    const char *js = "[\"asd \"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 8, 1,
          JSMN_STRING, "asd ", 0);
}

// y_string_space.json
static void y_string_space(void **state)
{
    (void)state; // unused
    const char *js = "\" \"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_STRING, " ", 0);
}

// y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json
static void y_string_surrogates_U_1D11E_MUSICAL_SYMBOL_G_CLEF(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD834\\uDd1e\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uD834\\uDd1e", 0);
}

// y_string_three-byte-utf-8.json
static void y_string_three_byte_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0821\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u0821", 0);
}

// y_string_two-byte-utf-8.json
static void y_string_two_byte_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0123\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u0123", 0);
}

// y_string_u+2028_line_sep.json
static void y_string_u_2028_line_sep(void **state)
{
    (void)state; // unused
    const char *js = "[\"‚Ä®\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "‚Ä®", 0);
}

// y_string_u+2029_par_sep.json
static void y_string_u_2029_par_sep(void **state)
{
    (void)state; // unused
    const char *js = "[\"‚Ä©\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "‚Ä©", 0);
}

// y_string_uescaped_newline.json
static void y_string_uescaped_newline(void **state)
{
    (void)state; // unused
    const char *js = "[\"new\\u000Aline\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 17, 1,
          JSMN_STRING, "new\\u000Aline", 0);
}

// y_string_uEscape.json
static void y_string_uEscape(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0061\\u30af\\u30EA\\u30b9\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 28, 1,
          JSMN_STRING, "\\u0061\\u30af\\u30EA\\u30b9", 0);
}

// y_string_unescaped_char_delete.json
static void y_string_unescaped_char_delete(void **state)
{
    (void)state; // unused
    const char *js = "[\"\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_STRING, "", 0);
}

// y_string_unicode_2.json
static void y_string_unicode_2(void **state)
{
    (void)state; // unused
    const char *js = "[\"‚çÇ„à¥‚çÇ\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 13, 1,
          JSMN_STRING, "‚çÇ„à¥‚çÇ", 0);
}

// y_string_unicodeEscapedBackslash.json
static void y_string_unicodeEscapedBackslash(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u005C\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u005C", 0);
}

// y_string_unicode_escaped_double_quote.json
static void y_string_unicode_escaped_double_quote(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u0022\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u0022", 0);
}

// y_string_unicode.json
static void y_string_unicode(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uA66D\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uA66D", 0);
}

// y_string_unicode_U+10FFFE_nonchar.json
static void y_string_unicode_U_10FFFE_nonchar(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uDBFF\\uDFFE\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uDBFF\\uDFFE", 0);
}

// y_string_unicode_U+1FFFE_nonchar.json
static void y_string_unicode_U_1FFFE_nonchar(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uD83F\\uDFFE\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 16, 1,
          JSMN_STRING, "\\uD83F\\uDFFE", 0);
}

// y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json
static void y_string_unicode_U_200B_ZERO_WIDTH_SPACE(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u200B\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u200B", 0);
}

// y_string_unicode_U+2064_invisible_plus.json
static void y_string_unicode_U_2064_invisible_plus(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\u2064\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\u2064", 0);
}

// y_string_unicode_U+FDD0_nonchar.json
static void y_string_unicode_U_FDD0_nonchar(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uFDD0\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uFDD0", 0);
}

// y_string_unicode_U+FFFE_nonchar.json
static void y_string_unicode_U_FFFE_nonchar(void **state)
{
    (void)state; // unused
    const char *js = "[\"\\uFFFE\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 10, 1,
          JSMN_STRING, "\\uFFFE", 0);
}

// y_string_utf8.json
static void y_string_utf8(void **state)
{
    (void)state; // unused
    const char *js = "[\"‚Ç¨ùÑû\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 11, 1,
          JSMN_STRING, "‚Ç¨ùÑû", 0);
}

// y_string_with_del_character.json
static void y_string_with_del_character(void **state)
{
    (void)state; // unused
    const char *js = "[\"aa\"]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 7, 1,
          JSMN_STRING, "aa", 0);
}

// y_structure_lonely_false.json
static void y_structure_lonely_false(void **state)
{
    (void)state; // unused
    const char *js = "false";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_PRIMITIVE, "false");
}

// y_structure_lonely_int.json
static void y_structure_lonely_int(void **state)
{
    (void)state; // unused
    const char *js = "42";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_PRIMITIVE, "42");
}

// y_structure_lonely_negative_real.json
static void y_structure_lonely_negative_real(void **state)
{
    (void)state; // unused
    const char *js = "-0.1";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_PRIMITIVE, "-0.1");
}

// y_structure_lonely_null.json
static void y_structure_lonely_null(void **state)
{
    (void)state; // unused
    const char *js = "null";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_PRIMITIVE, "null");
}

// y_structure_lonely_string.json
static void y_structure_lonely_string(void **state)
{
    (void)state; // unused
    const char *js = "\"asd\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_STRING, "asd", 0);
}

// y_structure_lonely_true.json
static void y_structure_lonely_true(void **state)
{
    (void)state; // unused
    const char *js = "true";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_PRIMITIVE, "true");
}

// y_structure_string_empty.json
static void y_structure_string_empty(void **state)
{
    (void)state; // unused
    const char *js = "\"\"";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_STRING, "", 0);
}

// y_structure_trailing_newline.json
static void y_structure_trailing_newline(void **state)
{
    (void)state; // unused
    const char *js = "[\"a\"]\n";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 5, 1,
          JSMN_STRING, "a", 0);
}

// y_structure_true_in_array.json
static void y_structure_true_in_array(void **state)
{
    (void)state; // unused
    const char *js = "[true]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY,  0, 6, 1,
          JSMN_PRIMITIVE, "true");
}

// y_structure_whitespace_array.json
static void y_structure_whitespace_array(void **state)
{
    (void)state; // unused
    const char *js = " [] ";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_ARRAY,  1, 3, 0);
}



void test_jsontestsuite_y(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(y_array_arraysWithSpaces, jsmn_setup),
        cmocka_unit_test_setup(y_array_empty, jsmn_setup),
        cmocka_unit_test_setup(y_array_empty_string, jsmn_setup),
        cmocka_unit_test_setup(y_array_false, jsmn_setup),
        cmocka_unit_test_setup(y_array_heterogeneous, jsmn_setup),
        cmocka_unit_test_setup(y_array_null, jsmn_setup),
        cmocka_unit_test_setup(y_array_with_1_and_newline, jsmn_setup),
        cmocka_unit_test_setup(y_array_with_leading_space, jsmn_setup),
        cmocka_unit_test_setup(y_array_with_several_null, jsmn_setup),
        cmocka_unit_test_setup(y_array_with_trailing_space, jsmn_setup),
        cmocka_unit_test_setup(y_number, jsmn_setup),
        cmocka_unit_test_setup(y_number_minus_zero, jsmn_setup),
        cmocka_unit_test_setup(y_number_0_eplus1, jsmn_setup),
        cmocka_unit_test_setup(y_number_0e1, jsmn_setup),
        cmocka_unit_test_setup(y_number_after_space, jsmn_setup),
        cmocka_unit_test_setup(y_number_double_close_to_zero, jsmn_setup),
        cmocka_unit_test_setup(y_number_int_with_exp, jsmn_setup),
        cmocka_unit_test_setup(y_number_negative_int, jsmn_setup),
        cmocka_unit_test_setup(y_number_negative_one, jsmn_setup),
        cmocka_unit_test_setup(y_number_negative_zero, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_capital_e, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_capital_e_neg_exp, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_capital_e_pos_exp, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_exponent, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_fraction_exponent, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_neg_exp, jsmn_setup),
        cmocka_unit_test_setup(y_number_real_pos_exponent, jsmn_setup),
        cmocka_unit_test_setup(y_number_simple_int, jsmn_setup),
        cmocka_unit_test_setup(y_number_simple_real, jsmn_setup),
        cmocka_unit_test_setup(y_object, jsmn_setup),
        cmocka_unit_test_setup(y_object_basic, jsmn_setup),
        cmocka_unit_test_setup(y_object_duplicated_key, jsmn_setup),
        cmocka_unit_test_setup(y_object_duplicated_key_and_value, jsmn_setup),
        cmocka_unit_test_setup(y_object_empty, jsmn_setup),
        cmocka_unit_test_setup(y_object_empty_key, jsmn_setup),
        cmocka_unit_test_setup(y_object_escaped_null_in_key, jsmn_setup),
        cmocka_unit_test_setup(y_object_extreme_numbers, jsmn_setup),
        cmocka_unit_test_setup(y_object_long_strings, jsmn_setup),
        cmocka_unit_test_setup(y_object_simple, jsmn_setup),
        cmocka_unit_test_setup(y_object_string_unicode, jsmn_setup),
        cmocka_unit_test_setup(y_object_with_newlines, jsmn_setup),
        cmocka_unit_test_setup(y_string_1_2_3_bytes_UTF8_sequences, jsmn_setup),
        cmocka_unit_test_setup(y_string_accepted_surrogate_pair, jsmn_setup),
        cmocka_unit_test_setup(y_string_accepted_surrogate_pairs, jsmn_setup),
        cmocka_unit_test_setup(y_string_allowed_escapes, jsmn_setup),
        cmocka_unit_test_setup(y_string_backslash_and_u_escaped_zero, jsmn_setup),
        cmocka_unit_test_setup(y_string_backslash_doublequotes, jsmn_setup),
        cmocka_unit_test_setup(y_string_comments, jsmn_setup),
        cmocka_unit_test_setup(y_string_double_escape_a, jsmn_setup),
        cmocka_unit_test_setup(y_string_double_escape_n, jsmn_setup),
        cmocka_unit_test_setup(y_string_escaped_control_character, jsmn_setup),
        cmocka_unit_test_setup(y_string_escaped_noncharacter, jsmn_setup),
        cmocka_unit_test_setup(y_string_in_array, jsmn_setup),
        cmocka_unit_test_setup(y_string_in_array_with_leading_space, jsmn_setup),
        cmocka_unit_test_setup(y_string_last_surrogates_1_and_2, jsmn_setup),
        cmocka_unit_test_setup(y_string_nbsp_uescaped, jsmn_setup),
        cmocka_unit_test_setup(y_string_nonCharacterInUTF8_U_10FFFF, jsmn_setup),
        cmocka_unit_test_setup(y_string_nonCharacterInUTF8_U_FFFF, jsmn_setup),
        cmocka_unit_test_setup(y_string_null_escape, jsmn_setup),
        cmocka_unit_test_setup(y_string_one_byte_utf8, jsmn_setup),
        cmocka_unit_test_setup(y_string_pi, jsmn_setup),
        cmocka_unit_test_setup(y_string_reservedCharacterInUTF8_U_1BFFF, jsmn_setup),
        cmocka_unit_test_setup(y_string_simple_ascii, jsmn_setup),
        cmocka_unit_test_setup(y_string_space, jsmn_setup),
        cmocka_unit_test_setup(y_string_surrogates_U_1D11E_MUSICAL_SYMBOL_G_CLEF, jsmn_setup),
        cmocka_unit_test_setup(y_string_three_byte_utf8, jsmn_setup),
        cmocka_unit_test_setup(y_string_two_byte_utf8, jsmn_setup),
        cmocka_unit_test_setup(y_string_u_2028_line_sep, jsmn_setup),
        cmocka_unit_test_setup(y_string_u_2029_par_sep, jsmn_setup),
        cmocka_unit_test_setup(y_string_uEscape, jsmn_setup),
        cmocka_unit_test_setup(y_string_uescaped_newline, jsmn_setup),
        cmocka_unit_test_setup(y_string_unescaped_char_delete, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_2, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_escaped_double_quote, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_10FFFE_nonchar, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_1FFFE_nonchar, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_200B_ZERO_WIDTH_SPACE, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_2064_invisible_plus, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_FDD0_nonchar, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicode_U_FFFE_nonchar, jsmn_setup),
        cmocka_unit_test_setup(y_string_unicodeEscapedBackslash, jsmn_setup),
        cmocka_unit_test_setup(y_string_utf8, jsmn_setup),
        cmocka_unit_test_setup(y_string_with_del_character, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_false, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_int, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_negative_real, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_null, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_string, jsmn_setup),
        cmocka_unit_test_setup(y_structure_lonely_true, jsmn_setup),
        cmocka_unit_test_setup(y_structure_string_empty, jsmn_setup),
        cmocka_unit_test_setup(y_structure_trailing_newline, jsmn_setup),
        cmocka_unit_test_setup(y_structure_true_in_array, jsmn_setup),
        cmocka_unit_test_setup(y_structure_whitespace_array, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("JSONTestSuite tests that should pass.", tests, NULL, NULL);
}


static void test_empty_01(void **state)
{
    (void)state; // unused
    const char *js = "{}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_OBJECT, 0, 2, 0);
}

static void test_empty_02(void **state)
{
    (void)state; // unused
    const char *js = "[]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 1), 1);
    tokeq(js, t, 1,
          JSMN_ARRAY, 0, 2, 0);
}

static void test_empty_03(void **state)
{
    (void)state; // unused
    const char *js = "[{},{}]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_ARRAY, 0, 7, 2,
          JSMN_OBJECT, 1, 3, 0,
          JSMN_OBJECT, 4, 6, 0);
}

void test_empty(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_empty_01, jsmn_setup),
        cmocka_unit_test_setup(test_empty_02, jsmn_setup),
        cmocka_unit_test_setup(test_empty_03, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test for empty JSON objects/arrays", tests, NULL, NULL);
}

static void test_object_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 7, 1,
          JSMN_STRING, "a", 1,
          JSMN_PRIMITIVE, "0");
}

static void test_object_02(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":[]}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 8, 1,
          JSMN_STRING, "a", 1,
          JSMN_ARRAY, 5, 7, 0);
}

static void test_object_03(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":{},\"b\":{}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, -1, -1, 2,
          JSMN_STRING, "a", 1,
          JSMN_OBJECT, -1, -1, 0,
          JSMN_STRING, "b", 1,
          JSMN_OBJECT, -1, -1, 0);
}

static void test_object_04(void **state)
{
    (void)state; // unused
    const char *js = "{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 7), 7);
    tokeq(js, t, 7,
          JSMN_OBJECT, -1, -1, 3,
          JSMN_STRING, "Day", 1,
          JSMN_PRIMITIVE, "26",
          JSMN_STRING, "Month", 1,
          JSMN_PRIMITIVE, "9",
          JSMN_STRING, "Year", 1,
          JSMN_PRIMITIVE, "12");
}

static void test_object_05(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 0, \"b\": \"c\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, -1, -1, 2,
          JSMN_STRING, "a", 1,
          JSMN_PRIMITIVE, "0",
          JSMN_STRING, "b", 1,
          JSMN_STRING, "c", 0);
}

#ifndef JSMN_PERMISSIVE
static void test_object_06(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\"\n0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_07(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\", 0}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_08(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": {2}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_09(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": {2: 3}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_10(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": {\"a\": 2 3}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_11(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_12(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 1, \"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_13(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\",\"b\":1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_14(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":1,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_15(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\":\"c\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_16(void **state)
{
    (void)state; // unused
    const char *js = "{,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}


static void test_object_17(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_18(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\" \"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_19(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\" ::::: \"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_20(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": [1 \"b\"]}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_21(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\"\"\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_22(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":1\"\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_23(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":1\"b\":1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_24(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"b\", \"c\":\"d\", {\"e\": \"f\"}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), (jsmnint_t)JSMN_ERROR_INVAL);
}
#endif

void test_object(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_object_01, jsmn_setup),
        cmocka_unit_test_setup(test_object_02, jsmn_setup),
        cmocka_unit_test_setup(test_object_03, jsmn_setup),
        cmocka_unit_test_setup(test_object_04, jsmn_setup),
        cmocka_unit_test_setup(test_object_05, jsmn_setup),
#ifndef JSMN_PERMISSIVE
        cmocka_unit_test_setup(test_object_06, jsmn_setup),
        cmocka_unit_test_setup(test_object_07, jsmn_setup),
        cmocka_unit_test_setup(test_object_08, jsmn_setup),
        cmocka_unit_test_setup(test_object_09, jsmn_setup),
        cmocka_unit_test_setup(test_object_10, jsmn_setup),
        cmocka_unit_test_setup(test_object_11, jsmn_setup),
        cmocka_unit_test_setup(test_object_12, jsmn_setup),
        cmocka_unit_test_setup(test_object_13, jsmn_setup),
        cmocka_unit_test_setup(test_object_14, jsmn_setup),
        cmocka_unit_test_setup(test_object_15, jsmn_setup),
        cmocka_unit_test_setup(test_object_16, jsmn_setup),

        cmocka_unit_test_setup(test_object_17, jsmn_setup),
        cmocka_unit_test_setup(test_object_18, jsmn_setup),
        cmocka_unit_test_setup(test_object_19, jsmn_setup),
        cmocka_unit_test_setup(test_object_20, jsmn_setup),
        cmocka_unit_test_setup(test_object_21, jsmn_setup),
        cmocka_unit_test_setup(test_object_22, jsmn_setup),
        cmocka_unit_test_setup(test_object_23, jsmn_setup),
        cmocka_unit_test_setup(test_object_24, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//   return cmocka_run_group_tests_name("test for JSON objects", tests, NULL, NULL);
}

static void test_array_01a(void **state)
{
    (void)state; // unused
    const char *js = "[10}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

static void test_array_01b(void **state)
{
    (void)state; // unused
    const char *js = "[10}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

static void test_array_02(void **state)
{
    (void)state; // unused
    const char *js = "[1,,3]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_array_03(void **state)
{
    (void)state; // unused
    const char *js = "[10]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_ARRAY, -1, -1, 1,
          JSMN_PRIMITIVE, "10");
}

static void test_array_04a(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

static void test_array_04b(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

static void test_array_05(void **state)
{
    (void)state; // unused
    const char *js = "[\"a\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

void test_array(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_array_01a, jsmn_setup),
        cmocka_unit_test_setup(test_array_01b, jsmn_setup),
        cmocka_unit_test_setup(test_array_02,  jsmn_setup),
        cmocka_unit_test_setup(test_array_03,  jsmn_setup),
        cmocka_unit_test_setup(test_array_04a, jsmn_setup),
        cmocka_unit_test_setup(test_array_04b, jsmn_setup),
        cmocka_unit_test_setup(test_array_05,  jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test for JSON arrays", tests, NULL, NULL);
}

static void test_primitive_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"boolVar\" : true }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "boolVar", 1,
          JSMN_PRIMITIVE, "true");
}

static void test_primitive_02(void **state)
{
    (void)state; // unused
    const char *js = "{\"boolVar\" : false }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "boolVar", 1,
          JSMN_PRIMITIVE, "false");
}

static void test_primitive_03(void **state)
{
    (void)state; // unused
    const char *js = "{\"nullVar\" : null }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "nullVar", 1,
          JSMN_PRIMITIVE, "null");
}

static void test_primitive_04(void **state)
{
    (void)state; // unused
    const char *js = "{\"intVar\" : 12}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "intVar", 1,
          JSMN_PRIMITIVE, "12");
}

static void test_primitive_05(void **state)
{
    (void)state; // unused
    const char *js = "{\"floatVar\" : 12.345}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "floatVar", 1,
          JSMN_PRIMITIVE, "12.345");
}

void test_primitive(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_primitive_01, jsmn_setup),
        cmocka_unit_test_setup(test_primitive_02, jsmn_setup),
        cmocka_unit_test_setup(test_primitive_03, jsmn_setup),
        cmocka_unit_test_setup(test_primitive_04, jsmn_setup),
        cmocka_unit_test_setup(test_primitive_05, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test primitive JSON data types", tests, NULL, NULL);
}

static void test_string_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"strVar\" : \"hello world\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "strVar", 1,
          JSMN_STRING, "hello world", 0);
}

static void test_string_02(void **state)
{
    (void)state; // unused
    const char *js = "{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "strVar", 1,
          JSMN_STRING, "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0);
}

static void test_string_03(void **state)
{
    (void)state; // unused
    const char *js = "{\"strVar\": \"\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "strVar", 1,
          JSMN_STRING, "", 0);
}

static void test_string_04(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"\\uAbcD\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "\\uAbcD", 0);
}

static void test_string_05(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"str\\u0000\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "str\\u0000", 0);
}

static void test_string_06(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"\\uFFFFstr\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "a", 1,
          JSMN_STRING, "\\uFFFFstr", 0);
}

static void test_string_07(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":[\"\\u0280\"]}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), 4);
    tokeq(js, t, 4,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "a", 1,
          JSMN_ARRAY, -1, -1, 1,
          JSMN_STRING, "\\u0280", 0);
}

static void test_string_08(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"str\\uFFGFstr\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_string_09(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":\"str\\u@FfF\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_string_10(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\":[\"\\u028\"]}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

void test_string(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_string_01, jsmn_setup),
        cmocka_unit_test_setup(test_string_02, jsmn_setup),
        cmocka_unit_test_setup(test_string_03, jsmn_setup),
        cmocka_unit_test_setup(test_string_04, jsmn_setup),
        cmocka_unit_test_setup(test_string_05, jsmn_setup),
        cmocka_unit_test_setup(test_string_06, jsmn_setup),
        cmocka_unit_test_setup(test_string_07, jsmn_setup),
        cmocka_unit_test_setup(test_string_08, jsmn_setup),
        cmocka_unit_test_setup(test_string_09, jsmn_setup),
        cmocka_unit_test_setup(test_string_10, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test string JSON data types", tests, NULL, NULL);
}

static void test_partial_string_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, t, 5), (jsmnint_t)JSMN_ERROR_PART);
    }
    assert_int_equal(jsmn_parse(&p, js, i, t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, -1, -1, 2,
          JSMN_STRING, "x", 1,
          JSMN_STRING, "va\\\\ue", 0,
          JSMN_STRING, "y", 1,
          JSMN_STRING, "value y", 0);
}

static void test_partial_string_02(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
    }
    assert_int_equal(jsmn_parse(&p, js, i, NULL, 0), 5);
}

void test_partial_string(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_partial_string_01, jsmn_setup),
        cmocka_unit_test_setup(test_partial_string_02, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test partial JSON string parsing", tests, NULL, NULL);
}

#ifndef JSMN_PERMISSIVE
static void test_partial_array_01(void **state)
{
    (void)state; // unused
    const char *js = "[ 1, true, [123, \"hello\"]]";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, t, 6), (jsmnint_t)JSMN_ERROR_PART);
    }
    assert_int_equal(jsmn_parse(&p, js, i, t, 6), 6);
    tokeq(js, t, 6,
          JSMN_ARRAY, 0, 26, 3,
          JSMN_PRIMITIVE, "1",
          JSMN_PRIMITIVE, "true",
          JSMN_ARRAY, 11, 25, 2,
          JSMN_PRIMITIVE, "123",
          JSMN_STRING, "hello", 0);
}

static void test_partial_array_02(void **state)
{
    (void)state; // unused
    const char *js = "[ 1, true, [123, \"hello\"]]";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, NULL, 0), (jsmnint_t)JSMN_ERROR_PART);
    }
    assert_int_equal(jsmn_parse(&p, js, i, NULL, 0), 6);
}
#endif

void test_partial_array(void)
{
    const struct CMUnitTest tests[] = {
#ifndef JSMN_PERMISSIVE
        cmocka_unit_test_setup(test_partial_array_01, jsmn_setup),
        cmocka_unit_test_setup(test_partial_array_02, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test partial array reading", tests, NULL, NULL);
}

static void test_array_nomem_01(void **state)
{
    (void)state; // unused
    const char *js = "  [ 1, true, [123, \"hello\"]]";

    int i;
    for (i = 0; i < 6; i++) {
        jsmn_init(&p);
        memset(t, 0, 6 * sizeof(jsmntok_t));
        assert_int_equal(jsmn_parse(&p, js, strlen(js), t, i), (jsmnint_t)JSMN_ERROR_NOMEM);
        assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), 6);
        tokeq(js, t, 6,
              JSMN_ARRAY, 2, 28, 3,
              JSMN_PRIMITIVE, "1",
              JSMN_PRIMITIVE, "true",
              JSMN_ARRAY, 13, 27, 2,
              JSMN_PRIMITIVE, "123",
              JSMN_STRING, "hello", 0);
    }
}

void test_array_nomem(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_array_nomem_01, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test array reading with a smaller number of tokens", tests, NULL, NULL);
}

#ifdef JSMN_PERMISSIVE
static void test_unquoted_keys_01(void **state)
{
    (void)state; // unused
    const char *js = "key1: \"value\"\nkey2 : 123";
#ifndef JSMN_MULTIPLE_JSON_FAIL
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), 4);
    tokeq(js, t, 4,
          JSMN_PRIMITIVE, "key1",
          JSMN_STRING, "value", 0,
          JSMN_PRIMITIVE, "key2",
          JSMN_PRIMITIVE, "123");
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
#endif
}
#endif

void test_unquoted_keys(void)
{
    const struct CMUnitTest tests[] = {
#ifdef JSMN_PERMISSIVE
        cmocka_unit_test_setup(test_unquoted_keys_01, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test unquoted keys (like in JavaScript)", tests, NULL, NULL);
}

static void test_issue_22(void **state)
{
    (void)state; // unused
    const char *js =
        "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
        "\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
        "\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
        "\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
        "\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
        "\"imageheight\":64, \"imagewidth\":160, \"margin\":0, \"name\":\"Tiles\", "
        "\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 }], "
        "\"tilewidth\":32, \"version\":1, \"width\":10 }";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 61);
}

static void test_issue_27(void **state)
{
    (void)state; // unused
    const char *js =
        "{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
#if defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), (jsmnint_t)JSMN_ERROR_INVAL);
#elif defined(JSMN_MULTIPLE_JSON)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), (jsmnint_t)JSMN_ERROR_PART);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, -1, -1, 2,
          JSMN_STRING, "name", 1,
          JSMN_STRING, "Jack", 0,
          JSMN_STRING, "age", 1,
          JSMN_PRIMITIVE, "27");
#endif
}

void test_issues(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_issue_22, jsmn_setup),
        cmocka_unit_test_setup(test_issue_27, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test issues", tests, NULL, NULL);
}

static void test_input_length_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 0}garbage";
    assert_int_equal(jsmn_parse(&p, js, 8, t, 10), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, -1, -1, 1,
          JSMN_STRING, "a", 1,
          JSMN_PRIMITIVE, "0");
}

void test_input_length(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_input_length_01, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test strings that are not null-terminated", tests, NULL, NULL);
}

static void test_count_01(void **state)
{
    (void)state; // unused
    const char *js = "{}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
}

static void test_count_02(void **state)
{
    (void)state; // unused
    const char *js = "[]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 1);
}

static void test_count_03(void **state)
{
    (void)state; // unused
    const char *js = "[[]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 2);
}

static void test_count_04(void **state)
{
    (void)state; // unused
    const char *js = "[[], []]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
}

static void test_count_05(void **state)
{
    (void)state; // unused
    const char *js = "[[], []]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 3);
}

static void test_count_06(void **state)
{
    (void)state; // unused
    const char *js = "[[], [[]], [[], []]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 7);
}

static void test_count_07(void **state)
{
    (void)state; // unused
    const char *js = "[\"a\", [[], []]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 5);
}

static void test_count_08(void **state)
{
    (void)state; // unused
    const char *js = "[[], \"[], [[]]\", [[]]]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 5);
}

static void test_count_09(void **state)
{
    (void)state; // unused
    const char *js = "[1, 2, 3]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 4);
}

static void test_count_10(void **state)
{
    (void)state; // unused
    const char *js = "[1, 2, [3, \"a\"], null]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), 7);
}


static void test_count_11(void **state)
{
    (void)state; // unused
    const char *js = "[}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

static void test_count_12(void **state)
{
    (void)state; // unused
    const char *js = "{]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), NULL, 0), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
}

void test_count(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_count_01, jsmn_setup),
        cmocka_unit_test_setup(test_count_02, jsmn_setup),
        cmocka_unit_test_setup(test_count_03, jsmn_setup),
        cmocka_unit_test_setup(test_count_04, jsmn_setup),
        cmocka_unit_test_setup(test_count_05, jsmn_setup),
        cmocka_unit_test_setup(test_count_06, jsmn_setup),
        cmocka_unit_test_setup(test_count_07, jsmn_setup),
        cmocka_unit_test_setup(test_count_08, jsmn_setup),
        cmocka_unit_test_setup(test_count_09, jsmn_setup),
        cmocka_unit_test_setup(test_count_10, jsmn_setup),

        cmocka_unit_test_setup(test_count_11, jsmn_setup),
        cmocka_unit_test_setup(test_count_12, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test tokens count estimation", tests, NULL, NULL);
}

#ifdef JSMN_PERMISSIVE
static void test_nonstrict_01(void **state)
{
    (void)state; // unused
    const char *js = "a: 0garbage";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_PRIMITIVE, "a",
          JSMN_PRIMITIVE, "0garbage");
}

static void test_nonstrict_02(void **state)
{
    (void)state; // unused
    const char *js = "Day : 26\nMonth : Sep\n\nYear: 12";
#ifndef JSMN_MULTIPLE_JSON_FAIL
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), 6);
    tokeq(js, t, 6,
          JSMN_PRIMITIVE, "Day",
          JSMN_PRIMITIVE, "26",
          JSMN_PRIMITIVE, "Month",
          JSMN_PRIMITIVE, "Sep",
          JSMN_PRIMITIVE, "Year",
          JSMN_PRIMITIVE, "12");
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), (jsmnint_t)JSMN_ERROR_INVAL);
#endif
}

static void test_nonstrict_03(void **state)
{
    (void)state; // unused
    //nested {s don't cause a parse error.
    const char *js = "\"key {1\": 1234";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 2,
          JSMN_STRING, "key {1", 1,
          JSMN_PRIMITIVE, "1234");
}

static void test_nonstrict_04(void **state)
{
    (void)state; // unused
    const char *js = "{a: 0garbage}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 2);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 13, 1,
          JSMN_PRIMITIVE, "a",
          JSMN_PRIMITIVE, "0garbage");
}
#endif

void test_nonstrict(void)
{
    const struct CMUnitTest tests[] = {
#ifdef JSMN_PERMISSIVE
        cmocka_unit_test_setup(test_nonstrict_01, jsmn_setup),
        cmocka_unit_test_setup(test_nonstrict_02, jsmn_setup),
        cmocka_unit_test_setup(test_nonstrict_03, jsmn_setup),
/*      cmocka_unit_test_setup(test_nonstrict_04, jsmn_setup), */
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test for non-strict mode", tests, NULL, NULL);
}

static void test_unmatched_brackets_01(void **state)
{
    (void)state; // unused
    const char *js = "\"key 1\": 1234}";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
# if defined(JSMN_PERMISSIVE_RULESET)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_UNMATCHED_BRACKETS);
# else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_INVAL);
# endif
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 1);
    tokeq(js, t, 1,
          JSMN_STRING, "key 1", 0);
#endif
}

static void test_unmatched_brackets_02(void **state)
{
    (void)state; // unused
    const char *js = "{\"key 1\": 1234";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_PART);
}

static void test_unmatched_brackets_03(void **state)
{
    (void)state; // unused
    const char *js = "{\"key 1\": 1234}}";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 15, 1,
          JSMN_STRING, "key 1", 1,
          JSMN_PRIMITIVE, "1234");
#endif
}

static void test_unmatched_brackets_04(void **state)
{
    (void)state; // unused
    const char *js = "\"key 1\"}: 1234";
#if defined(JSMN_MULTIPLE_JSON) || defined(JSMN_MULTIPLE_JSON_FAIL)
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
#else
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), 1);
    tokeq(js, t, 1,
          JSMN_STRING, "key 1", 0);
#endif
}

static void test_unmatched_brackets_05(void **state)
{
    (void)state; // unused
    const char *js = "{\"key {1\": 1234}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
          JSMN_OBJECT, 0, 16, 1,
          JSMN_STRING, "key {1", 1,
          JSMN_PRIMITIVE, "1234");
}

static void test_unmatched_brackets_06(void **state)
{
    (void)state; // unused
    const char *js = "{\"key 1\":{\"key 2\": 1234}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_PART);
}

void test_unmatched_brackets(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_unmatched_brackets_01, jsmn_setup),
        cmocka_unit_test_setup(test_unmatched_brackets_02, jsmn_setup),
        cmocka_unit_test_setup(test_unmatched_brackets_03, jsmn_setup),
        cmocka_unit_test_setup(test_unmatched_brackets_04, jsmn_setup),
        cmocka_unit_test_setup(test_unmatched_brackets_05, jsmn_setup),
        cmocka_unit_test_setup(test_unmatched_brackets_06, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test for unmatched brackets", tests, NULL, NULL);
}

static void test_object_key_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"key\": 1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), 3);
    tokeq(js, t, 3,
        JSMN_OBJECT, 0, 10, 1,
        JSMN_STRING, "key", 1,
        JSMN_PRIMITIVE, "1");
}

#ifndef JSMN_PERMISSIVE
static void test_object_key_02(void **state)
{
    (void)state; // unused
    const char *js = "{true: 1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_key_03(void **state)
{
    (void)state; // unused
    const char *js = "{1: 1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_key_04(void **state)
{
    (void)state; // unused
    const char *js = "{{\"key\": 1}: 2}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_key_05(void **state)
{
    (void)state; // unused
    const char *js = "{[1,2]: 2}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}
#endif

void test_object_key(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_object_key_01, jsmn_setup),
#ifndef JSMN_PERMISSIVE
        cmocka_unit_test_setup(test_object_key_02, jsmn_setup),
        cmocka_unit_test_setup(test_object_key_03, jsmn_setup),
        cmocka_unit_test_setup(test_object_key_04, jsmn_setup),
        cmocka_unit_test_setup(test_object_key_05, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests) / sizeof(struct CMUnitTest);
//  return cmocka_run_group_tests_name("test for non-strict mode", tests, NULL, NULL);
}

#if !defined(JSMN_PERMISSIVE)
# if defined(JSMN_LOW_MEMORY)
#  define JSMN_TEST_GROUP "jsmn_test_default_low_memory"
# elif defined(JSMN_MULTIPLE_JSON_FAIL)
#  define JSMN_TEST_GROUP "jsmn_test_default_mult_json_fail"
# else
#  define JSMN_TEST_GROUP "jsmn_test_default"
# endif
#else
# if defined(JSMN_LOW_MEMORY)
#  define JSMN_TEST_GROUP "jsmn_test_permissive_low_memory"
# elif defined(JSMN_MULTIPLE_JSON_FAIL)
#  define JSMN_TEST_GROUP "jsmn_test_permissive_mult_json_fail"
# else
#  define JSMN_TEST_GROUP "jsmn_test_permissive"
# endif
#endif

int main(void)
{
    struct CMUnitTest *tests = cur_test = calloc(512, sizeof(struct CMUnitTest));
    test_empty();          // test for a empty JSON objects/arrays
    test_object();         // test for a JSON objects
    test_array();          // test for a JSON arrays
    test_primitive();      // test primitive JSON data types
    test_string();         // test string JSON data types

    test_partial_string(); // test partial JSON string parsing
    test_partial_array();  // test partial array reading
    test_array_nomem();    // test array reading with a smaller number of tokens
    test_unquoted_keys();  // test unquoted keys (like in JavaScript)
    test_input_length();   // test strings that are not null-terminated
    test_issues();         // test issues
    test_count();          // test tokens count estimation
    test_nonstrict();      // test for non-strict mode
    test_unmatched_brackets(); // test for unmatched brackets
    test_object_key();     // test for key type

    test_jsontestsuite_i();
    test_jsontestsuite_n();
    test_jsontestsuite_y();

    return _cmocka_run_group_tests(JSMN_TEST_GROUP, tests, total_tests, NULL, NULL);
//  return cmocka_run_group_tests(tests, NULL, NULL);
}
