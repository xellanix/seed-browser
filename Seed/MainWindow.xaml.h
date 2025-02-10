#pragma once

#include "Tab.g.h"
#include "TabItem.g.h"
#include "MainWindow.g.h"
#include "Utilities.h"
#include <chrono>
#include "winrt/Microsoft.Web.WebView2.Core.h"

namespace winrt
{
    namespace wf = Windows::Foundation;
    namespace wfc = Windows::Foundation::Collections;
    namespace muc = Microsoft::UI::Composition;
    namespace mud = Microsoft::UI::Dispatching;
    namespace mux = Microsoft::UI::Xaml;
    namespace muxc = Microsoft::UI::Xaml::Controls;
    namespace muxd = Microsoft::UI::Xaml::Data;
    namespace muxma = Microsoft::UI::Xaml::Media::Animation;
    namespace muxn = Microsoft::UI::Xaml::Navigation;
    namespace muw = Microsoft::UI::Windowing;
    namespace mwwv2c = Microsoft::Web::WebView2::Core;
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

    struct TabItem : TabItemT<TabItem>
    {
        TabItem(hstring const& initialUrl) : m_currentUrl(initialUrl)
        {
            static uint32_t count = 0;
            m_id = count++;

            m_content = muxc::WebView2();
            m_content.Source(wf::Uri(initialUrl));

            m_content.EnsureCoreWebView2Async();
            m_initRevoker = m_content.CoreWebView2Initialized(auto_revoke, { this, &TabItem::CoreInitialize });

            InitializeSleepTimer();
        }

        uint32_t Id() const { return m_id; }

        mux::UIElement Content() const { return m_content; }

        hstring SavedStateURL() const { return m_savedUrl; }

        hstring CurrentURL() const { return m_currentUrl; }
        void CurrentURL(hstring const& value)
        {
            if (m_currentUrl != value)
            {
                m_currentUrl = value;
                m_content.Source(wf::Uri(value));
                m_title = m_content.CoreWebView2().DocumentTitle();

                RaisePropertyChanged(L"CurrentURL");
            }
        }

        hstring Title() const { return m_title; }
        void Title(hstring const& value)
        {
            if (m_title != value)
            {
                m_title = value;
                RaisePropertyChanged(L"Title");
            }
        }

        hstring FaviconUri() const { return m_favicon; }
        void FaviconUri(hstring const& value)
        {
            if (m_favicon != value)
            {
                m_favicon = value;
                RaisePropertyChanged(L"FaviconUri");
            }
        }

        void ActivateTab()
        {
            if (m_content && m_content.CoreWebView2().Source() == L"about:blank" && !m_savedUrl.empty())
            {
                m_content.Source(wf::Uri(m_savedUrl));
                m_currentUrl = m_savedUrl;
            }

            // Restart the sleep timer.
            if (m_sleepTimer)
                m_sleepTimer.Start();
        }

        void CloseTab()
        {
            m_content.Close();
            m_sleepRevoker.revoke();
            m_initRevoker.revoke();
            m_titleRevoker.revoke();
            m_sleepTimer = nullptr;
            m_content = nullptr;
        }

        event_token PropertyChanged(muxd::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }

    private:
        void CoreInitialize(muxc::WebView2 const&, muxc::CoreWebView2InitializedEventArgs const&)
        {
            Title(m_content.CoreWebView2().DocumentTitle());

            FaviconUri(m_content.CoreWebView2().FaviconUri());

            m_titleRevoker = m_content.CoreWebView2().DocumentTitleChanged(auto_revoke, { this, &TabItem::TitleChanged });
        }

        void TitleChanged(mwwv2c::CoreWebView2 const& sender, wf::IInspectable const&)
        {
            Title(sender.DocumentTitle());
            FaviconUri(m_content.CoreWebView2().FaviconUri());
        }

        void InitializeSleepTimer()
        {
            auto dispatcher{ mud::DispatcherQueue::GetForCurrentThread() };

            m_sleepTimer = dispatcher.CreateTimer();
            m_sleepTimer.Interval(std::chrono::minutes(30));
            m_sleepRevoker = m_sleepTimer.Tick(auto_revoke, [this](auto&&, auto&&)
            {
                // Logger::LogInfo(L"Tab auto-sleep triggered for URL: " + m_currentUrl);
                SleepTab();
            });
            m_sleepTimer.Start();
        }

        void SleepTab()
        {
            // Save the current URL.
            m_savedUrl = m_currentUrl;

            if (m_content)
                m_content.Source(wf::Uri(L"about:blank"));
        }

        void RaisePropertyChanged(hstring const& propertyName)
        {
            m_propertyChanged(*this, muxd::PropertyChangedEventArgs(propertyName));
        }

        uint32_t m_id = 0;
        muxc::WebView2 m_content{ nullptr };
        hstring m_savedUrl;
        hstring m_currentUrl;
        hstring m_title;
        hstring m_favicon = L"https://google.com/favicon.ico";

        mud::DispatcherQueueTimer m_sleepTimer{ nullptr };
        mud::DispatcherQueueTimer::Tick_revoker m_sleepRevoker;
        muxc::WebView2::CoreWebView2Initialized_revoker m_initRevoker;
        mwwv2c::CoreWebView2::DocumentTitleChanged_revoker m_titleRevoker;
        winrt::event<muxd::PropertyChangedEventHandler> m_propertyChanged;
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        winrt::mux::FrameworkElement m_rootElement{ nullptr };
        wfc::IObservableVector<Seed::Tab> m_favTabs{ winrt::single_threaded_observable_vector<Seed::Tab>() };
        wfc::IObservableVector<Seed::TabItem> m_tabItems{ winrt::single_threaded_observable_vector<Seed::TabItem>() };

        void SetModernAppTitleBar();
        bool GetCurrentTheme();

        void CreateNewTab(winrt::hstring const& url);

        bool SearchTabID(uint32_t search, uint32_t& id);

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
        
        wfc::IObservableVector<Seed::TabItem> TabItems() const { return m_tabItems; }
        void TabItems(wfc::IObservableVector<Seed::TabItem> const& value)
        {
            if (m_tabItems != value)
            {
                m_tabItems = value;
            }
        }

        void Window_SizeChanged(wf::IInspectable const& sender, mux::WindowSizeChangedEventArgs const& args);

        void SwitchThemeByClick(muxc::SplitButton const& sender, muxc::SplitButtonClickEventArgs const& e);
        void SwitchThemeByMenu(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);

        void MinimizeClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void MaximizeClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void CloseClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void Grid_Loaded(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        
        void NewTabRequested(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void RemoveTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void SelectedTabChanged(wf::IInspectable const& sender, muxc::SelectionChangedEventArgs const& e);
    };
}

namespace winrt::Seed::factory_implementation
{
    struct Tab : TabT<Tab, implementation::Tab>
    {};
    
    struct TabItem : TabItemT<TabItem, implementation::TabItem>
    {};
    
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {};
}
