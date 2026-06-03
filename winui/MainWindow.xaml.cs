using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using OpenclawGuard.Pages;

namespace OpenclawGuard;

public sealed partial class MainWindow : Window
{
    public MainWindow() => InitializeComponent();

    private void NavView_SelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args)
    {
        if (args.SelectedItem is NavigationViewItem item && item.Tag is string tag)
            Navigate(tag);
    }

    private void NavView_ItemInvoked(NavigationView sender, NavigationViewItemInvokedEventArgs args)
    {
        if (args.InvokedItemContainer?.Tag is string tag)
            Navigate(tag);
    }

    private void Navigate(string tag)
    {
        Page page = tag switch
        {
            "dashboard" => new DashboardPage(),
            "gateway"   => new GatewayPage(),
            "guard"     => new GuardPage(),
            "updates"   => new UpdatesPage(),
            "settings"  => new SettingsPage(),
            _           => new DashboardPage()
        };
        ContentFrame.Navigate(page.GetType(), null);
    }
}
