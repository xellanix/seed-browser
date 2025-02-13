#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#if __has_include("Tab.g.cpp")
#include "Tab.g.cpp"
#endif
#if __has_include("TabItem.g.cpp")
#include "TabItem.g.cpp"
#endif

#pragma region WinUI Headers

#include "microsoft.ui.xaml.window.h" // for IWindowNative
#include "winrt/Microsoft.UI.Windowing.h"
#include "winrt/Microsoft.UI.Input.h"
#include "winrt/Microsoft.UI.Interop.h"
#include "winrt/Microsoft.UI.Xaml.Hosting.h"
#include "winrt/Microsoft.UI.Xaml.Media.Animation.h"
#include "winrt/Microsoft.UI.Xaml.Input.h"

#pragma endregion

#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Seed::implementation
{
    MainWindow::MainWindow()
    {
        SetModernAppTitleBar();
    }

    double MainWindow::GetScaleAdjustment()
    {
        Microsoft::UI::WindowId windowId =
            Microsoft::UI::GetWindowIdFromWindow(xellanix::desktop::WindowHandle);

        muw::DisplayArea displayArea = muw::DisplayArea::GetFromWindowId(windowId, muw::DisplayAreaFallback::Nearest);
        auto hMonitor = Microsoft::UI::GetMonitorFromDisplayId(displayArea.DisplayId());

        // Get DPI.
        UINT dpiX, dpiY;
        int result = GetDpiForMonitor(hMonitor, MDT_DEFAULT, &dpiX, &dpiY);
        if (result != S_OK)
        {
            return 1.0;
        }

        auto scaleFactorPercent = dpiX / 96.0;
        return scaleFactorPercent;
    }

    void MainWindow::SetModernAppTitleBar()
    {
        auto windowNative{ this->try_as<::IWindowNative>() };
        winrt::check_bool(windowNative);
        windowNative->get_WindowHandle(&xellanix::desktop::WindowHandle);

        muw::AppWindow appWindow = this->AppWindow();
        appWindow.SetIcon(L"Assets/icon.ico");

        if (muw::AppWindowTitleBar::IsCustomizationSupported())
        {
            auto titleBar{ appWindow.TitleBar() };

            titleBar.ExtendsContentIntoTitleBar(true);
            titleBar.SetDragRectangles(
                {
                    Windows::Graphics::RectInt32{ 0, 0, static_cast<int32_t>(this->Bounds().Width), static_cast<int32_t>(12 * GetScaleAdjustment()) }
                }
            );

            if (auto presenter = appWindow.Presenter().try_as<muw::OverlappedPresenter>())
            {
                presenter.SetBorderAndTitleBar(true, false);
            }
        }
        else
        {
            // Title bar customization using these APIs is currently
            // supported only on Windows 11. In other cases, hide
            // the custom title bar element.
            AppTitleBar().Visibility(mux::Visibility::Collapsed);

            // Show alternative UI for any functionality in
            // the title bar, such as search.
        }
    }

    bool MainWindow::GetCurrentTheme()
    {
        bool isCurrentDarkMode = false;

        if (m_rootElement == nullptr) m_rootElement = this->Content().try_as<winrt::mux::FrameworkElement>();

        if (auto theme{ m_rootElement.RequestedTheme() }; theme == mux::ElementTheme::Dark)
        {
            isCurrentDarkMode = true;
        }
        else if (theme == mux::ElementTheme::Light)
        {
            isCurrentDarkMode = false;
        }
        else
        {
            if (auto currentApp{ mux::Application::Current() })
            {
                isCurrentDarkMode = currentApp.RequestedTheme() == mux::ApplicationTheme::Dark;
            }
        }

        return isCurrentDarkMode;
    }

    void MainWindow::CreateNewTab(winrt::hstring const& url)
    {
        namespace muxh = Microsoft::UI::Xaml::Hosting;

        // Create a new tab that hosts a CEF-based browser.
        auto tab = make_self<implementation::TabItem>(2ui8, url);
        tab->RegisterURLBox(URLBox());
        tab->RegisterGoBackButton(GoBackBtn());
        tab->RegisterGoForwardButton(GoForwardBtn());

        m_tabItems.Append(*tab);
        RegulerTabList().SelectedIndex(m_tabItems.Size() - 1);
    }

    bool MainWindow::SearchTabID(uint32_t search, uint8_t type, uint32_t& id)
    {
        if (type > 2ui8) return false;

        // 2 = reguler, 1 = pinned, 0 = favorite
        auto const& vec{ type == 2ui8 ? m_tabItems : (type == 1ui8 ? m_pinnedTabItems : m_tiledTabItems) };

        uint32_t l = 0, h = vec.Size() - 1;
        if (vec.GetAt(l).Id() == search)
        {
            id = l;
            return true;
        }

        while (l <= h)
        {
            uint32_t m = l + ((h - l) / 2);

            if (vec.GetAt(m).Id() == search)
            {
                id = m;
                return true;
            }
            else if (vec.GetAt(m).Id() < search) l = m + 1;
            else if (vec.GetAt(m).Id() > search) h = m - 1;
        }

        return false;
    }

    bool MainWindow::TryGoBack()
    {
        if (m_selectedTab)
        {
            if (auto view{ m_selectedTab.Content().try_as<muxc::WebView2>() })
            {
                if (view.CanGoBack())
                {
                    view.GoBack();
                    return true;
                }
            }
        }

        return false;
    }

    bool MainWindow::TryGoForward()
    {
        if (m_selectedTab)
        {
            if (auto view{ m_selectedTab.Content().try_as<muxc::WebView2>() })
            {
                if (view.CanGoForward())
                {
                    view.GoForward();
                    return true;
                }
            }
        }

        return false;
    }

    void MainWindow::CloseTab(Seed::TabItem const& item)
    {
        auto type{ item.Type() };

        uint32_t index = 0;
        SearchTabID(item.Id(), type, index);

        item.CloseTab();
        
        if (type == 2ui8) m_tabItems.RemoveAt(index);
        else if (type == 1ui8) m_pinnedTabItems.RemoveAt(index);
        else if (type == 0ui8) m_tiledTabItems.RemoveAt(index);
        
        ContentGrid().Children().Clear();
    }

    void MainWindow::UpdateSelectedItem(wf::IInspectable const& item)
    {
        if (auto tab{ item.try_as<Seed::TabItem>() })
        {
            tab.ActivateTab();

            ContentGrid().Children().Clear();
            ContentGrid().Children().Append(tab.Content());

            URLBox().Text(tab.CurrentURL());

            if (m_selectedTab)
            {
                if (auto type{ m_selectedTab.Type() }; type != tab.Type())
                {
                    uint32_t index = 0;
                    SearchTabID(m_selectedTab.Id(), type, index);

                    switch (type)
                    {
                        case 0ui8:
                            TiledTabList().Deselect(index);
                            break;
                        case 1ui8:
                            PinnedTabList().DeselectRange(muxd::ItemIndexRange(index, 1));
                            break;
                        case 2ui8:
                            RegulerTabList().DeselectRange(muxd::ItemIndexRange(index, 1));
                            break;
                    }
                }
            }

            m_selectedTab.PutToSleepTab();
            m_selectedTab = tab;
        }
    }

    void MainWindow::Grid_Loaded(wf::IInspectable const&, mux::RoutedEventArgs const&) {}

    void MainWindow::Grid_PointerPressed(wf::IInspectable const&, muxi::PointerRoutedEventArgs const& e)
    {
        if (e.GetCurrentPoint(nullptr).Properties().IsXButton1Pressed())
        {
            e.Handled(!TryGoBack());
        }
        else if (e.GetCurrentPoint(nullptr).Properties().IsXButton2Pressed())
        {
            e.Handled(!TryGoForward());
        }
    }

    void MainWindow::NewTabRequested(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        CreateNewTab(L"https://google.com");
    }

    void MainWindow::RemoveTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        const auto context{ sender.as<muxc::Button>().DataContext().as<Seed::TabItem>() };
        if (!context) return;

        CloseTab(context);
    }

    void MainWindow::SelectedTabChanged(wf::IInspectable const& sender, muxc::SelectionChangedEventArgs const&)
    {
        if (auto lv{ sender.try_as<muxc::ListView>() })
        {
            UpdateSelectedItem(lv.SelectedItem());
        }
    }

    void MainWindow::SelectedTileTabChanged(muxc::ItemsView const& sender, muxc::ItemsViewSelectionChangedEventArgs const&)
    {
        UpdateSelectedItem(sender.SelectedItem());
    }

    void MainWindow::GoBackClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        TryGoBack();
    }

    void MainWindow::GoForwardClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        TryGoForward();
    }

    void MainWindow::ReloadClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        if (auto view{ m_selectedTab.Content().try_as<muxc::WebView2>() })
        {
            view.Reload();
        }
    }

    void MainWindow::URLBox_KeyDown(wf::IInspectable const&, muxi::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            hstring uri{ xu::normalize_uri(URLBox().Text()) };

            if (m_selectedTab) m_selectedTab.Navigate(uri);
            else CreateNewTab(uri);

            Content().Focus(mux::FocusState::Programmatic);
        }
    }

    void MainWindow::FocusURLBarInvoked(muxi::KeyboardAccelerator const&, muxi::KeyboardAcceleratorInvokedEventArgs const&)
    {
        URLBox().Focus(mux::FocusState::Programmatic);
    }

    wf::IAsyncAction MainWindow::RenameTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        if (auto btn{ sender.try_as<muxc::MenuFlyoutItem>() })
        {
            auto item{ btn.DataContext().try_as<Seed::TabItem>() };
            if (item && RegulerTabContext()) {}
            else if (item = m_selectedTab) {}
            else return;

            muxc::ContentDialog dialog{};
            dialog.XamlRoot(Content().XamlRoot());
            dialog.Title(box_value(L"Rename tab"));
            dialog.PrimaryButtonText(L"Save");
            dialog.CloseButtonText(L"Cancel");
            dialog.DefaultButton(muxc::ContentDialogButton::Primary);

            Seed::RenameTabDialog content{};
            content.TabUrl(item.CurrentURL());
            content.NewTitle(item.Title());

            dialog.Content(content);

            auto res = co_await dialog.ShowAsync();
            if (res == muxc::ContentDialogResult::Primary)
            {
                // save and set
                item.Title(content.NewTitle());
            }
        }
    }

    void MainWindow::PinTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        if (auto btn{ sender.try_as<muxc::MenuFlyoutItem>() })
        {
            auto item{ btn.DataContext().try_as<Seed::TabItem>() };
            if (item && RegulerTabContext()) {}
            else if (item = m_selectedTab) {}
            else return;

            auto before{ item.Type() };
            uint32_t index = 0;
            SearchTabID(item.Id(), before, index);

            if (before == 1ui8)
            {
                // Unpin
                item.Type(2ui8);
                m_tabItems.Append(item);
                RegulerTabList().SelectedIndex(m_tabItems.Size() - 1);

                m_pinnedTabItems.RemoveAt(index);
            }
            else
            {
                item.Type(1ui8);
                m_pinnedTabItems.Append(item);
                PinnedTabList().SelectedIndex(m_pinnedTabItems.Size() - 1);

                if (before == 2ui8) m_tabItems.RemoveAt(index);
                else if (before == 0ui8) m_tiledTabItems.RemoveAt(index);
            }
        }
    }

    void MainWindow::TileTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        if (auto btn{ sender.try_as<muxc::MenuFlyoutItem>() })
        {
            auto item{ btn.DataContext().try_as<Seed::TabItem>() };
            if (item && RegulerTabContext()) {}
            else if (item = m_selectedTab) {}
            else return;

            auto before{ item.Type() };
            uint32_t index = 0;
            SearchTabID(item.Id(), before, index);

            item.Type(0ui8);
            m_tiledTabItems.Append(item);
            TiledTabList().Select(m_tiledTabItems.Size() - 1);
            if (m_tiledTabItems.Size() <= 1) TiledTabList().UpdateLayout();
            
            if (before == 2ui8) m_tabItems.RemoveAt(index);
            else if (before == 1ui8) m_pinnedTabItems.RemoveAt(index);
        }
    }

    void MainWindow::CloseTabClicked(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        if (auto btn{ sender.try_as<muxc::MenuFlyoutItem>() })
        {
            auto item{ btn.DataContext().try_as<Seed::TabItem>() };
            if (item && RegulerTabContext()) {}
            else if (item = m_selectedTab) {}
            else return;

            CloseTab(item);
        }
    }

    void MainWindow::Window_SizeChanged(wf::IInspectable const&, mux::WindowSizeChangedEventArgs const& args)
    {
        if (muw::AppWindowTitleBar::IsCustomizationSupported())
            this->AppWindow().TitleBar().SetDragRectangles(
            {
                Windows::Graphics::RectInt32{ 0, 0, static_cast<int32_t>(args.Size().Width), static_cast<int32_t>(12 * GetScaleAdjustment()) }
            }
        );
    }

    void MainWindow::SwitchThemeByClick(muxc::SplitButton const&, muxc::SplitButtonClickEventArgs const&)
    {
        if (auto icon{ ThemeIcon() })
        {
            auto theme{ mux::ElementTheme::Light };
            hstring style{ L"" };

            if (GetCurrentTheme())
            {
                theme = mux::ElementTheme::Light;
                style = L"\uE706";
            }
            else
            {
                theme = mux::ElementTheme::Dark;
                style = L"\uE708";
            }

            m_rootElement.RequestedTheme(theme);
            icon.Glyph(style);
        }
    }

    void MainWindow::SwitchThemeByMenu(wf::IInspectable const& sender, mux::RoutedEventArgs const&)
    {
        if (auto item{ sender.try_as<muxc::MenuFlyoutItem>() })
        {
            auto mode = item.Text();

            auto ChangeTheme = [&](mux::ElementTheme theme, mux::ElementTheme target, hstring const& style)
            {
                if (theme == target) return;

                m_rootElement.RequestedTheme(target);

                if (auto icon{ ThemeIcon() })
                {
                    icon.Glyph(style);
                }
            };

            if (auto theme = m_rootElement.RequestedTheme(); mode == L"System Default")
            {
                ChangeTheme(theme, mux::ElementTheme::Default, L"\uF08C");
            }
            else if (mode == L"Dark")
            {
                ChangeTheme(theme, mux::ElementTheme::Dark, L"\uE708");
            }
            else if (mode == L"Light")
            {
                ChangeTheme(theme, mux::ElementTheme::Light, L"\uE706");
            }
        }
    }

    void MainWindow::MinimizeClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        if (auto presenter = this->AppWindow().Presenter().try_as<muw::OverlappedPresenter>())
        {
            presenter.Minimize();
        }
    }

    void MainWindow::MaximizeClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        if (auto presenter = this->AppWindow().Presenter().try_as<muw::OverlappedPresenter>())
        {
            if (presenter.State() == muw::OverlappedPresenterState::Maximized)
                presenter.Restore();
            else
                presenter.Maximize();
        }
    }

    void MainWindow::CloseClicked(wf::IInspectable const&, mux::RoutedEventArgs const&)
    {
        this->Close();
    }
}
