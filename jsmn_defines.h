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
#  define JSMN_PARENT_LINKS     /*!< Adds a parent field to the token  */
# endif
# ifndef JSMN_NEXT_SIBLING
#  define JSMN_NEXT_SIBLING     /*!< Adds a next_sibling field to the token */
# endif

#else

# ifndef JSMN_SHORT_TOKENS
#  define JSMN_SHORT_TOKENS     /*!< Changes the tokens field size from a uint32_t to a uint16_t */
# endif                         /*   This reduces the maximum possible json string length from 4,294,967,295 to 65,535 */

#endif

#ifdef JSMN_PERMISSIVE                  /*!< Allow all PERMISSIVE options */
# ifndef JSMN_PERMISSIVE_KEY
#  define JSMN_PERMISSIVE_KEY           /*!< Allows PRIMITIVEs to be OBJECT KEYs */
# endif
# ifndef JSMN_PERMISSIVE_STRINGS        /*!< Allows STRINGs to  */
#  define JSMN_PERMISSIVE_STRINGS
# endif
# ifndef JSMN_PERMISSIVE_PRIMITIVES
#  define JSMN_PERMISSIVE_PRIMITIVES    /*!< Allows PRIMITIVEs to be any contiguous value */
# endif
# ifndef JSMN_MULTIPLE_JSON
#  define JSMN_MULTIPLE_JSON            /*!< Allows multiple json objects in a complete buffer */
# endif
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
