#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "isxTest.h"

#include <QCoreApplication>

#include <vector>

void additionalUsage()
{
    std::cout << "  -p, --test-data-path         set path to test data in testDataPath global\n";
}

int main( int argc, char const * argv[] )
{
    std::vector<char const *> argv_mod;
    bool showUsage = false;
    bool findData = true;

    for (int i = 0; i < argc; ++i)
    {
        std::string a(argv[i]);
        if (a == "-p" || a == "--test-data-path")
        {
            ++i;
            a = argv[i];
            if (!isDataPath(a))
            {
                std::cerr << "Test data path does not exist: " << a << "\n";
                return 1;
            }
            g_resources["testDataPath"] = a;
            findData = false;
        }
        else
        {
            if (a == "-?" || a == "-h" || a == "--help")
            {
                showUsage = true;
            }
            argv_mod.push_back(argv[i]);
        }
    }

    if (findData)
    {
        std::string dataPath = "test_data";
        if (!findDataPath(dataPath, 5))
        {
            std::cerr << "Could not find test data path.\n";
            return 1;
        }
        g_resources["testDataPath"] = dataPath;
    }

    g_resources["unitTestDataPath"] = g_resources["testDataPath"] + "/unit_test";
    g_resources["realTestDataPath"] = g_resources["testDataPath"] + "/real";
    g_resources["benchDataPath"] = g_resources["testDataPath"] + "/bench";

    // needed for starting event loop on private threads (via QThread::exec)
    int one = 1;
    QCoreApplication a(one, (char **)&argv_mod[0]);

    int result = Catch::Session().run(int(argv_mod.size()), &argv_mod[0]);
    showUsage |= (result == Catch::ResultWas::OfType::Unknown);
    if (showUsage)
    {
        additionalUsage();
    }
    return result;
}
