#pragma once

#include "Tab.g.h"
#include "MainWindow.g.h"
#include "Utilities.h"

namespace winrt
{
    namespace wf = Windows::Foundation;
    namespace wfc = Windows::Foundation::Collections;
    namespace muc = Microsoft::UI::Composition;
    namespace mux = Microsoft::UI::Xaml;
    namespace muxc = Microsoft::UI::Xaml::Controls;
    namespace muxd = Microsoft::UI::Xaml::Data;
    namespace muxma = Microsoft::UI::Xaml::Media::Animation;
    namespace muxn = Microsoft::UI::Xaml::Navigation;
    namespace muw = Microsoft::UI::Windowing;
}

namespace winrt::Seed::implementation
{
    struct Tab : TabT<Tab>
    {
        Tab() = default;

        hstring URL() const { return m_url; }
        void URL(hstring const& value) { UpdateValue(m_url, value, L"URL"); }

        hstring Title() const { return m_title; }
        void Title(hstring const& value) { UpdateValue(m_title, value, L"Title"); }

        hstring Favicon() const { return m_favicon; }
        void Favicon(hstring const& value) { UpdateValue(m_favicon, value, L"Favicon"); }

        event_token PropertyChanged(muxd::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }

    private:
        hstring m_url;
        hstring m_title;
        hstring m_favicon;
        winrt::event<muxd::PropertyChangedEventHandler> m_propertyChanged;

        template <typename T>
        void UpdateValue(T& target, T const& value, hstring const& prop)
        {
            if (target != value)
            {
                target = value;
                RaisePropertyChanged(prop);
            }
        }

    protected:
        void RaisePropertyChanged(hstring const& propertyName)
        {
            m_propertyChanged(*this, muxd::PropertyChangedEventArgs(propertyName));
        }
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        winrt::mux::FrameworkElement m_rootElement{ nullptr };
        wfc::IObservableVector<Seed::Tab> m_favTabs{ winrt::single_threaded_observable_vector<Seed::Tab>() };

        void SetModernAppTitleBar();
        bool GetCurrentTheme();

    public:
        MainWindow();

        wfc::IObservableVector<Seed::Tab> FavoritTabs() const { return m_favTabs; }
        void FavoritTabs(wfc::IObservableVector<Seed::Tab> const& value)
        {
            if (m_favTabs != value)
            {
                m_favTabs = value;
            }
        }

        void Window_SizeChanged(wf::IInspectable const& sender, winrt::mux::WindowSizeChangedEventArgs const& args);

        void SwitchThemeByClick(winrt::muxc::SplitButton const& sender, winrt::muxc::SplitButtonClickEventArgs const& e);
        void SwitchThemeByMenu(wf::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);

        void MinimizeClicked(wf::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
        void MaximizeClicked(wf::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
        void CloseClicked(wf::IInspectable const& sender, winrt::mux::RoutedEventArgs const& e);
    };
}

namespace winrt::Seed::factory_implementation
{
    struct Tab : TabT<Tab, implementation::Tab>
    {};
    
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {};
}
