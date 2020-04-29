#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdio.h>

#include "jsmn.h"

jsmn_parser p;
jsmntok_t t[128];
int total_tests = 0;
void *cur_test = NULL;

int jsmn_setup(void **state)
{
    (void)state; // unused
    jsmn_init(&p);
    memset(t, 0, 128 * sizeof(jsmntok_t));

    return 0;
}

/**
 * @brief va_arg token comparison
 *
 * @param[in] s json string
 * @param[in] t pointer to jsmn tokens
 * @param[in] numtok number of tokens
 * @param[in] ap p_ap:...
 */
void vtokeq(const char *s, jsmntok_t *t, int numtok, va_list ap)
{
    if (numtok <= 0)
        return;

    int i, start, end, size = -1;
    int type;
    char *value = NULL;

    for (i = 0; i < numtok; i++) {
        type = va_arg(ap, int);
        switch (type) {
            case JSMN_STRING:
                value = va_arg(ap, char *);
                size = va_arg(ap, int);
                start = end = -1;
                break;
            case JSMN_PRIMITIVE:
                value = va_arg(ap, char *);
                start = end = size = -1;
                break;
            default:
                value = NULL;
                start = va_arg(ap, int);
                end = va_arg(ap, int);
                size = va_arg(ap, int);
                break;
        }
        if (t[i].type != type) {
            fail_msg("token %d type is %d, not %d", i, t[i].type, type);
        }

        if (start != -1 && end != -1) {
            if (t[i].start != start) {
                fail_msg("token %d start is %d, not %d", i, t[i].start, start);
            }
            if (t[i].end != end) {
                fail_msg("token %d end is %d, not %d", i, t[i].end, end);
            }
        }

        if (size != -1 && t[i].size != size) {
            fail_msg("token %d size is %d, not %d", i, t[i].size, size);
        }

        if (s != NULL && value != NULL) {
            const char *p = s + t[i].start;
            if (strlen(value) != t[i].end - t[i].start ||
                    strncmp(p, value, t[i].end - t[i].start) != 0) {
                fail_msg("token %d value is %.*s, not %s",
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
void tokeq(const char *s, jsmntok_t *tokens, int numtok, ...)
{
    va_list args;
    va_start(args, numtok);
    vtokeq(s, tokens, numtok, args);
    va_end(args);
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test for empty JSON objects/arrays", tests, NULL, NULL);
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

#ifdef JSMN_STRICT
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
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_09(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": {2: 3}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_10(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": {\"a\": 2 3}}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), (jsmnint_t)JSMN_ERROR_INVAL);
}

/* FIXME test_object 11-16 */
static void test_object_11(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{\"a\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_12(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{\"a\": 1, \"b\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_13(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{\"a\",\"b\":1}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_14(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{\"a\":1,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_15(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{\"a\":\"b\":\"c\"}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_object_16(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "{,}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_INVAL);
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
#ifdef JSMN_STRICT
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
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//   return cmocka_run_group_tests_name("test for JSON objects", tests, NULL, NULL);
}

/* FIXME test_array 1,2,5 */
static void test_array_01(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "[10}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_array_02(void **state)
{
    (void)state; // unused
    skip();
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

static void test_array_04(void **state)
{
    (void)state; // unused
    const char *js = "{\"a\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_array_05(void **state)
{
    (void)state; // unused
    skip();
    const char *js = "[\"a\": 1]";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

void test_array(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_array_01, jsmn_setup),
        cmocka_unit_test_setup(test_array_02, jsmn_setup),
        cmocka_unit_test_setup(test_array_03, jsmn_setup),
        cmocka_unit_test_setup(test_array_04, jsmn_setup),
        cmocka_unit_test_setup(test_array_05, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test for JSON arrays", tests, NULL, NULL);
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test primitive JSON data types", tests, NULL, NULL);
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
    const char *js = "{{\"a\":[\"\\u028\"]}";
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test string JSON data types", tests, NULL, NULL);
}

static void test_partial_string_01(void **state)
{
    (void)state; // unused
    const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, t, 5), (jsmnint_t)JSMN_ERROR_PART);
    }

    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 5), 5);
    tokeq(js, t, 5,
          JSMN_OBJECT, -1, -1, 2,
          JSMN_STRING, "x", 1,
          JSMN_STRING, "va\\\\ue", 0,
          JSMN_STRING, "y", 1,
          JSMN_STRING, "value y", 0);
}

void test_partial_string(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_partial_string_01, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test partial JSON string parsing", tests, NULL, NULL);
}

#ifdef JSMN_STRICT
static void test_partial_array_01(void **state)
{
    (void)state; // unused
    const char *js = "[ 1, true, [123, \"hello\"]]";

    int i;
    for (i = 1; i < strlen(js); i++) {
        assert_int_equal(jsmn_parse(&p, js, i, t, 6), (jsmnint_t)JSMN_ERROR_PART);
    }

    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), 6);
    tokeq(js, t, 6,
          JSMN_ARRAY, -1, -1, 3,
          JSMN_PRIMITIVE, "1",
          JSMN_PRIMITIVE, "true",
          JSMN_ARRAY, -1, -1, 2,
          JSMN_PRIMITIVE, "123",
          JSMN_STRING, "hello", 0);
}
#endif

void test_partial_array(void)
{
    const struct CMUnitTest tests[] = {
#ifdef JSMN_STRICT
        cmocka_unit_test_setup(test_partial_array_01, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test partial array reading", tests, NULL, NULL);
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
              JSMN_ARRAY, -1, -1, 3,
              JSMN_PRIMITIVE, "1",
              JSMN_PRIMITIVE, "true",
              JSMN_ARRAY, -1, -1, 2,
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test array reading with a smaller number of tokens", tests, NULL, NULL);
}

static void test_unquoted_keys_01(void **state)
{
    (void)state; // unused
#ifndef JSMN_STRICT
    const char *js = "key1: \"value\"\nkey2 : 123";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), 4);
    tokeq(js, t, 4,
          JSMN_PRIMITIVE, "key1",
          JSMN_STRING, "value", 0,
          JSMN_PRIMITIVE, "key2",
          JSMN_PRIMITIVE, "123");
#endif
}

void test_unquoted_keys(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_unquoted_keys_01, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test unquoted keys (like in JavaScript)", tests, NULL, NULL);
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test strings that are not null-terminated", tests, NULL, NULL);
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
    assert_in_range(jsmn_parse(&p, js, strlen(js), t, 128), 0, 128);
}

static void test_issue_27(void **state)
{
    (void)state; // unused
    const char *js =
        "{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 8), (jsmnint_t)JSMN_ERROR_PART);
}

void test_issues(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_issue_22, jsmn_setup),
        cmocka_unit_test_setup(test_issue_27, jsmn_setup),
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test issues", tests, NULL, NULL);
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
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test tokens count estimation", tests, NULL, NULL);
}

#ifndef JSMN_STRICT
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
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 6), 6);
    tokeq(js, t, 6,
          JSMN_PRIMITIVE, "Day",
          JSMN_PRIMITIVE, "26",
          JSMN_PRIMITIVE, "Month",
          JSMN_PRIMITIVE, "Sep",
          JSMN_PRIMITIVE, "Year",
          JSMN_PRIMITIVE, "12");
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
#endif

void test_nonstrict(void)
{
    const struct CMUnitTest tests[] = {
#ifndef JSMN_STRICT
        cmocka_unit_test_setup(test_nonstrict_01, jsmn_setup),
        cmocka_unit_test_setup(test_nonstrict_02, jsmn_setup),
        cmocka_unit_test_setup(test_nonstrict_03, jsmn_setup),
#endif
    };

    memcpy(cur_test, tests, sizeof(tests));
    cur_test += sizeof(tests);
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test for non-strict mode", tests, NULL, NULL);
}

static void test_unmatched_brackets_01(void **state)
{
    (void)state; // unused
    const char *js = "\"key 1\": 1234}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 2), (jsmnint_t)JSMN_ERROR_INVAL);
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
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
}

static void test_unmatched_brackets_04(void **state)
{
    (void)state; // unused
    const char *js = "\"key 1\"}: 1234";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 3), (jsmnint_t)JSMN_ERROR_INVAL);
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
    const char *js = "{{\"key 1\": 1234}";
    assert_int_equal(jsmn_parse(&p, js, strlen(js), t, 4), (jsmnint_t)JSMN_ERROR_PART);
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
    total_tests += sizeof(tests)/sizeof(struct CMUnitTest);
//     return cmocka_run_group_tests_name("test for unmatched brackets", tests, NULL, NULL);
}

int main(void)
{
    struct CMUnitTest *tests = cur_test = calloc(128, sizeof(struct CMUnitTest));
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

    return _cmocka_run_group_tests("jsmn_test", tests, total_tests, NULL, NULL);
//     return cmocka_run_group_tests(tests, NULL, NULL);
}
