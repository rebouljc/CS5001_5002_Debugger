#pragma once

#include "StartPage.g.h"

namespace winrt::EasyDebuggerTest::implementation
{
    struct StartPage : StartPageT<StartPage>
    {
        StartPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        
    };
}

namespace winrt::EasyDebuggerTest::factory_implementation
{
    struct StartPage : StartPageT<StartPage, implementation::StartPage>
    {
    };
}
