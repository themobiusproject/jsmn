#ifndef JSMN_DEFINES
#define JSMN_DEFINES

/* If nothing is defined, the default definitions are JSMN_PARENT_LINKS and   *
 *   JSMN_NEXT_SIBLING with a jsmntok_t field size of 4 bytes (unsigned int). *
 * JSMN_LOW_MEMORY doesn't enable JSMN_PARENT_LINKS or JSMN_NEXT_SIBLING and  *
 *   reduces the jsmntok_t field size to 2 bytes (unsigned short).            *
 * JSMN_PERMISSIVE enables all of the permissive rules.                       *
 *   JSMN_PERMISSIVE_KEY allows PRIMITIVEs to be OBJECT KEYs                  *
 *   JSMN_PERMISSIVE_PRIMITIVES allows PRIMITIVEs to be any contiguous value  *
 *     that doesn't include a special character (`{}[],:"`)                   *
 *   JSMN_MULTIPLE_JSON allows multiple json objects in a complete buffer     *
 *     Note: A single malformed json object will return a jsmnerr_t rather    *
 *           than the number of tokens parsed.                                */

#ifndef JSMN_API
# ifdef JSMN_STATIC
#  define JSMN_API static
# else
#  define JSMN_API extern
# endif
#endif

#ifndef JSMN_LOW_MEMORY

# ifndef JSMN_PARENT_LINKS
/*!
 * @def JSMN_PARENT_LINKS
 * @brief Adds a parent field to the token
 *
 * This simplifies the post-processing of tokens by adding a link to the id of
 * a token's parent.
 * This is enabled by default and highly recommended.
 */
#  define JSMN_PARENT_LINKS
# endif
# ifndef JSMN_NEXT_SIBLING
/*!
 * @def JSMN_PARENT_LINKS
 * @brief Adds a next_sibling field to the token
 *
 * This simplifies the post-processing of tokens by adding a link to the id of
 * a token's next sibling.
 * This is enabled by default and highly recommended.
 */
#  define JSMN_NEXT_SIBLING
# endif

#else

# ifndef JSMN_SHORT_TOKENS
/*!
 * @def JSMN_SHORT_TOKENS
 * @brief Changes the tokens field size from a uint32_t to a uint16_t
 *
 * This reduces the jsmntok_t size by half by changing jsmntok_t field sizes
 * from an unsigned int to an unsigned short. NOTE: This reduces the maximum
 * possible json string length from 4,294,967,295 to 65,535 minus the size of
 * jsmnerr.
 */
#  define JSMN_SHORT_TOKENS
# endif

#endif

/*!
 * @def JSMN_PERMISSIVE
 * @brief Enables all PERMISSIVE definitions
 *
 * Enables JSMN_PERMISSIVE_KEY,JSMN_PERMISSIVE_PRIMITIVES, and
 * JSMN_MULTIPLE_JSON
 */
#ifdef JSMN_PERMISSIVE
# ifndef JSMN_PERMISSIVE_KEY
/*!
 * @def JSMN_PERMISSIVE_KEY
 * @brief Allows PRIMITIVEs to be OBJECT KEYs
 */
#  define JSMN_PERMISSIVE_KEY
# endif
# ifndef JSMN_PERMISSIVE_PRIMITIVES
/*!
 * @def JSMN_PERMISSIVE_PRIMITIVES
 * @brief Allows PRIMITIVEs to be any contiguous value
 *
 * This allows PRIMIVITEs to be any contiguous value that does not contain a
 * character that has a special meaning to json (`{}[]",:`)
 */
#  define JSMN_PERMISSIVE_PRIMITIVES
# endif
# ifndef JSMN_MULTIPLE_JSON
/*!
 * @def JSMN_MULTIPLE_JSON
 * @brief Allows multiple json objects in a complete buffer
 *
 * This allows jsmn to parse multiple json objects in a single buffer. Please
 * note that if a single json object is malformed jsmn_parse will return with
 * an error.
 */
#  define JSMN_MULTIPLE_JSON
# endif
#endif

/*!
 * @def JSMN_MULTIPLE_JSON_FAIL
 * @brief Allows one and only one json objest to be in a complete buffer
 *
 * This will only allow one complete json object to be in a buffer. NOTE: If any other
 * non-whitespace character after a successful json object jsmn_parse will
 * return with an error.
 */
#ifdef JSMN_MULTIPLE_JSON_FAIL
# undef JSMN_MULTIPLE_JSON
#endif

#endif // JSMN_DEFINES

#if (defined(__linux__) || defined(__APPLE__) || defined(ARDUINO))
# define JSMN_EXPORT __attribute__((visibility("default")))
# define JSMN_LOCAL  __attribute__((visibility("hidden")))
#elif (defined(_WIN32))
# define JSMN_EXPORT __declspec(dllexport)
# define JSMN_LOCAL
#else
# define JSMN_EXPORT
# define JSMN_LOCAL
#endif
