import pandas as pd
import numpy as np

def load_data(file_path):
    """ Load historical price data from a CSV file """
    data = pd.read_csv(file_path)
    data['Date'] = pd.to_datetime(data['Date'])
    data.set_index('Date', inplace=True)
    return data

def calculate_sma(data, window):
    """ Calculate Simple Moving Average (SMA) """
    return data['Close'].rolling(window=window).mean()

def generate_signals(data, short_window, long_window):
    """ Generate buy and sell signals based on SMA crossovers """
    data['Short_SMA'] = calculate_sma(data, short_window)
    data['Long_SMA'] = calculate_sma(data, long_window)
    data['Signal'] = 0
    data['Signal'][short_window:] = np.where(data['Short_SMA'][short_window:] > data['Long_SMA'][short_window:], 1, 0)
    data['Position'] = data['Signal'].diff()
    return data

def backtest_strategy(data):
    """ Simulate trading based on SMA crossover signals """
    initial_capital = float(10000.0)
    positions = pd.DataFrame(index=data.index).fillna(0.0)
    portfolio = pd.DataFrame(index=data.index).fillna(0.0)

    # Buy a 100 shares
    positions['Crypto'] = 100 * data['Signal']
    portfolio['holdings'] = positions.multiply(data['Close'], axis=0)
    portfolio['cash'] = initial_capital - (positions.diff() * data['Close']).cumsum()
    portfolio['total'] = portfolio['cash'] + portfolio['holdings']
    portfolio['returns'] = portfolio['total'].pct_change()
    return portfolio

def main():
    # Load data
    data = load_data('crypto_price_data.csv')
    
    # Settings for the SMA windows
    short_window = 20
    long_window = 50

    # Generate trading signals
    trading_data = generate_signals(data, short_window, long_window)
    
    # Backtest the strategy
    portfolio = backtest_strategy(trading_data)
    
    # Print the final output
    print(portfolio.tail())
    trading_data[['Close', 'Short_SMA', 'Long_SMA']].plot()

if __name__ == "__main__":
    main()
