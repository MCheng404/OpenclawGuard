using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace OpenclawGuard.Pages;

public sealed partial class DashboardPage : Page
{
    public record StatItem(string Label, string Value, SolidColorBrush Color);

    public ObservableCollection<StatItem> Stats { get; } = new()
    {
        new("Gateway", "--", new SolidColorBrush(Microsoft.UI.Colors.DodgerBlue)),
        new("Guards", "0", new SolidColorBrush(Microsoft.UI.Colors.DodgerBlue)),
        new("Updates", "0", new SolidColorBrush(Microsoft.UI.Colors.DodgerBlue)),
    };

    public DashboardPage() => InitializeComponent();
}
