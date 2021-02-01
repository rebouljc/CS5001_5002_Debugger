#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include <string>


using namespace winrt;


namespace winrt::EasyDebuggerTest::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
    }

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainPage::NavView_ItemInvoked(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args)
    {
        
        if (args.InvokedItemContainer())
        {
            Frame().Navigate(xaml_typename<EasyDebuggerTest::StartPage>(), args);
        }
        
    }

 

   
}
