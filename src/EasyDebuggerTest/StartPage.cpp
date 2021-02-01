#include "pch.h"
#include "StartPage.h"
#if __has_include("StartPage.g.cpp")
#include "StartPage.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EasyDebuggerTest::implementation
{
    StartPage::StartPage()
    {
        InitializeComponent();
    }

    int32_t StartPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void StartPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

  
}
