#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#if __has_include("Tab.g.cpp")
#include "Tab.g.cpp"
#endif

#pragma region WinUI Headers

#include "microsoft.ui.xaml.window.h" // for IWindowNative
#include "winrt/Microsoft.UI.Windowing.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Microsoft.UI.Xaml.Media.Animation.h"

#pragma endregion

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Seed::implementation
{
    MainWindow::MainWindow()
    {
        SetModernAppTitleBar();

        for (int i = 0; i < 5; i++)
        {
            auto item = winrt::make_self<implementation::Tab>();
            item->URL(L"https://google.com");
            item->Title(L"Google");
            item->Favicon(L"https://raw.githubusercontent.com/DennisSuitters/LibreICONS/refs/heads/master/svg/libre-gui-bookmark.svg");
            m_favTabs.Append(*item);
        }
    }

    void MainWindow::SetModernAppTitleBar()
    {
        auto windowNative{ this->try_as<::IWindowNative>() };
        winrt::check_bool(windowNative);
        windowNative->get_WindowHandle(&Xellanix::Desktop::WindowHandle);

        muw::AppWindow appWindow = this->AppWindow();

        if (muw::AppWindowTitleBar::IsCustomizationSupported())
        {
            auto titleBar{ appWindow.TitleBar() };

            titleBar.ExtendsContentIntoTitleBar(true);
            titleBar.SetDragRectangles(
                {
                    Windows::Graphics::RectInt32{ 0, 0, static_cast<int32_t>(this->Bounds().Width), 12 }
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

    void MainWindow::Window_SizeChanged(wf::IInspectable const&, mux::WindowSizeChangedEventArgs const& args)
    {
        if (muw::AppWindowTitleBar::IsCustomizationSupported())
            this->AppWindow().TitleBar().SetDragRectangles(
            {
                Windows::Graphics::RectInt32{ 0, 0, static_cast<int32_t>(args.Size().Width), 12 }
            }
        );
    }

    void MainWindow::SwitchThemeByClick(winrt::muxc::SplitButton const&, winrt::muxc::SplitButtonClickEventArgs const&)
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

    void MainWindow::SwitchThemeByMenu(wf::IInspectable const& sender, winrt::mux::RoutedEventArgs const&)
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
