#include "App.h"
using namespace Core;

// TODO: Calling DebugLog from python only logs to the console.

int main()
{
    App app({ 1920, 1080, 4, 5, "ModernOpenGL", true, false, 60 });
    app.Run();
    return 0;
}
