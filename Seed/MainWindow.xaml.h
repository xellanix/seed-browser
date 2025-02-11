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
    namespace muxi = Microsoft::UI::Xaml::Input;
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
        TabItem(uint8_t type, hstring const& initialUrl) : m_type(type), m_currentUrl(initialUrl)
        {
            static uint32_t count = 0;
            m_id = count++;

            m_content = muxc::WebView2();
            m_content.Source(wf::Uri(initialUrl));

            m_content.EnsureCoreWebView2Async();
            m_initRevoker = m_content.CoreWebView2Initialized(auto_revoke, { this, &TabItem::CoreInitialize });

            m_backToken = m_content.RegisterPropertyChangedCallback(muxc::WebView2::CanGoBackProperty(), { this, &TabItem::WebView2DepsChanged });
            m_forwardToken = m_content.RegisterPropertyChangedCallback(muxc::WebView2::CanGoForwardProperty(), { this, &TabItem::WebView2DepsChanged });

            InitializeSleepTimer();
        }

        uint32_t Id() const { return m_id; }

        uint8_t Type() const { return m_type; }
        void Type(uint8_t value)
        {
            if (m_type != value)
            {
                m_type = value;
                RaisePropertyChanged(L"Type");
            }
        }

        mux::UIElement Content() const { return m_content; }

        hstring SavedStateURL() const { return m_savedUrl; }

        hstring CurrentURL() const { return m_currentUrl; }
        void CurrentURL(hstring const& value)
        {
            if (m_currentUrl != value)
            {
                m_currentUrl = value;
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
            if (m_content && m_content.Source().ToString() == L"about:blank" && !m_savedUrl.empty())
            {
                m_content.Source(wf::Uri(m_savedUrl));
                m_currentUrl = m_savedUrl;
            }

            // Restart the sleep timer.
            if (m_sleepTimer)
                m_sleepTimer.Start();
        }

        void Navigate(hstring const& url)
        {
            CurrentURL(url);
            m_content.Source(wf::Uri(url));
            m_title = m_content.CoreWebView2().DocumentTitle();
        }

        void CloseTab()
        {
            m_faviconRevoker.revoke();
            m_titleRevoker.revoke();
            m_sourceRevoker.revoke();
            m_sleepRevoker.revoke();
            m_initRevoker.revoke();
            
            m_content.UnregisterPropertyChangedCallback(muxc::WebView2::CanGoBackProperty(), m_backToken);
            m_content.UnregisterPropertyChangedCallback(muxc::WebView2::CanGoForwardProperty(), m_forwardToken);

            m_content.Close();

            m_sleepTimer = nullptr;
            m_content = nullptr;
            m_urlBox = nullptr;
            m_backBtn = nullptr;
            m_forwardBtn = nullptr;
        }

        void RegisterURLBox(muxc::TextBox const& urlBox) { m_urlBox = urlBox; }

        void RegisterGoBackButton(muxc::Control const& btn) { m_backBtn = btn; }
        void RegisterGoForwardButton(muxc::Control const& btn) { m_forwardBtn = btn; }

        event_token PropertyChanged(muxd::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }

    private:
        void CoreInitialize(muxc::WebView2 const&, muxc::CoreWebView2InitializedEventArgs const&)
        {
            auto core{ m_content.CoreWebView2() };

            core.Profile().PreferredColorScheme(mwwv2c::CoreWebView2PreferredColorScheme::Dark);

            Title(core.DocumentTitle());
            FaviconUri(core.FaviconUri());

            m_sourceRevoker = core.SourceChanged(auto_revoke, { this, &TabItem::SourceChanged });
            m_faviconRevoker = core.FaviconChanged(auto_revoke, { this, &TabItem::FaviconChanged });
            m_titleRevoker = core.DocumentTitleChanged(auto_revoke, { this, &TabItem::TitleChanged });
        }

        void FaviconChanged(mwwv2c::CoreWebView2 const& sender, wf::IInspectable) { FaviconUri(sender.FaviconUri()); }

        void SourceChanged(mwwv2c::CoreWebView2 const& sender, mwwv2c::CoreWebView2SourceChangedEventArgs const&)
        {
            if (m_urlBox) m_urlBox.Text(sender.Source());
            CurrentURL(sender.Source());
        }

        void TitleChanged(mwwv2c::CoreWebView2 const& sender, wf::IInspectable const&) { Title(sender.DocumentTitle()); }

        void WebView2DepsChanged(DependencyObject const& sender, DependencyProperty const& e)
        {
            if (e == muxc::WebView2::CanGoBackProperty()) m_backBtn.IsEnabled(unbox_value<bool>(sender.GetValue(e)));
            else if (e == muxc::WebView2::CanGoForwardProperty()) m_forwardBtn.IsEnabled(unbox_value<bool>(sender.GetValue(e)));
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
        uint8_t m_type = 2;
        muxc::WebView2 m_content{ nullptr };
        hstring m_savedUrl;
        hstring m_currentUrl;
        hstring m_title;
        hstring m_favicon = L"https://google.com/favicon.ico";

        muxc::TextBox m_urlBox{ nullptr };
        muxc::Control m_backBtn{ nullptr };
        muxc::Control m_forwardBtn{ nullptr };

        mud::DispatcherQueueTimer m_sleepTimer{ nullptr };
        mud::DispatcherQueueTimer::Tick_revoker m_sleepRevoker;
        muxc::WebView2::CoreWebView2Initialized_revoker m_initRevoker;
        mwwv2c::CoreWebView2::SourceChanged_revoker m_sourceRevoker;
        mwwv2c::CoreWebView2::FaviconChanged_revoker m_faviconRevoker;
        mwwv2c::CoreWebView2::DocumentTitleChanged_revoker m_titleRevoker;

        int64_t m_backToken;
        int64_t m_forwardToken;

        winrt::event<muxd::PropertyChangedEventHandler> m_propertyChanged;
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        winrt::mux::FrameworkElement m_rootElement{ nullptr };
        wfc::IObservableVector<Seed::Tab> m_favTabs{ winrt::single_threaded_observable_vector<Seed::Tab>() };
        wfc::IObservableVector<Seed::TabItem> m_tabItems{ winrt::single_threaded_observable_vector<Seed::TabItem>() };

        Seed::TabItem m_selectedTab{ nullptr };

        double GetScaleAdjustment();
        void SetModernAppTitleBar();
        bool GetCurrentTheme();

        void CreateNewTab(winrt::hstring const& url);

        bool SearchTabID(uint32_t search, uint32_t& id);

        bool TryGoBack();
        bool TryGoForward();

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
        void Grid_PointerPressed(wf::IInspectable const& sender, muxi::PointerRoutedEventArgs const& e);
        
        void NewTabRequested(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void RemoveTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void SelectedTabChanged(wf::IInspectable const& sender, muxc::SelectionChangedEventArgs const& e);

        void GoBackClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void GoForwardClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void ReloadClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const& e);
        void URLBox_KeyDown(wf::IInspectable const& sender, muxi::KeyRoutedEventArgs const& e);
        
        void FocusURLBarInvoked(muxi::KeyboardAccelerator const& sender, muxi::KeyboardAcceleratorInvokedEventArgs const& args);
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
