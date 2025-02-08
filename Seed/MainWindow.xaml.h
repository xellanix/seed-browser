#pragma once

#include "MainWindow.g.h"
#include "Utilities.h"

namespace winrt
{
    namespace muc = Microsoft::UI::Composition;
    namespace mux = Microsoft::UI::Xaml;
    namespace muxc = Microsoft::UI::Xaml::Controls;
    namespace muxma = Microsoft::UI::Xaml::Media::Animation;
    namespace muxn = Microsoft::UI::Xaml::Navigation;
    namespace muw = Microsoft::UI::Windowing;
}

namespace winrt::Seed::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        winrt::mux::FrameworkElement m_rootElement{ nullptr };

        void SetModernAppTitleBar();
        bool GetCurrentTheme();

    public:
        MainWindow();
        void Window_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::mux::WindowSizeChangedEventArgs const& args);

        void SwitchThemeByClick(winrt::muxc::SplitButton const& sender, winrt::muxc::SplitButtonClickEventArgs const& e);
        void SwitchThemeByMenu(winrt::Windows::Foundation::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);

        void MinimizeClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
        void MaximizeClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
        void CloseClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
    };
}

namespace winrt::Seed::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
