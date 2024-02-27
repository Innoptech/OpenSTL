#ifndef OPENSTL_TESTS_TESTUTILS_H
#define OPENSTL_TESTS_TESTUTILS_H
#include "assets/assets_path.h"
#include <string>

namespace openstl {
    namespace testutils {
        enum class TESTOBJECT {
            KEY, BALL, WASHER
        };

        inline std::string getTestObjectPath(TESTOBJECT obj) {
            std::string basename{};
            switch (obj) {
                default:
                    basename = "KEY.STL";
                    break;
                case TESTOBJECT::BALL:
                    basename = "BALL.STL";
                    break;
                case TESTOBJECT::WASHER:
                    basename = "WASHER.STL";
                    break;
            }
            return OPENSTL_TEST_ASSETSDIR + basename;
        }
    } //namespace testutils
} //namespace openstl
#endif //OPENSTL_TESTS_TESTUTILS_H
