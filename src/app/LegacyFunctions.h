#ifndef APP_LEGACY_FUNCTIONS_H
#define APP_LEGACY_FUNCTIONS_H

#include <vector>
#include <string>

namespace v8unpack {

/**
 * @brief Legacy functions for backward compatibility
 *
 * These functions are temporary bridges until full command system is implemented.
 * They will be replaced by proper Command objects in the future.
 */

/**
 * @brief Show usage information
 * @param argv Command line arguments
 * @return Exit code
 */
int usage(std::vector<std::string>& argv);

/**
 * @brief Show version information
 * @param argv Command line arguments
 * @return Exit code
 */
int version(std::vector<std::string>& argv);

/**
 * @brief Execute batch scripts
 * @param argv Command line arguments
 * @return Exit code
 */
int bat(std::vector<std::string>& argv);

/**
 * @brief Show usage examples
 * @param argv Command line arguments
 * @return Exit code
 */
int example(std::vector<std::string>& argv);

} // namespace v8unpack

#endif // APP_LEGACY_FUNCTIONS_H
