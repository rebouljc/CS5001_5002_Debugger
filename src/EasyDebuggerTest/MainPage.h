#pragma once

#include "MainPage.g.h"


namespace winrt::EasyDebuggerTest::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        
        MainPage();
        
        void NavView_ItemInvoked(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        int32_t MyProperty();
        void MyProperty(int32_t value);
        
        
        
    };
}

namespace winrt::EasyDebuggerTest::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
