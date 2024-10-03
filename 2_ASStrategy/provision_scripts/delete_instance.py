import sqlite3
import sys
import os

print(f"Python script located at: {os.getcwd()}")
os.chdir(os.path.expanduser('~/ss/bt/db'))
print(f"Python script navigated to: {os.getcwd()}")
if len(sys.argv) > 2 and os.path.exists('strategy.db'):
    INSTANCE_NAME = sys.argv[1]
    STRATEGY_NAME = sys.argv[2]
    
    conn = sqlite3.connect('strategy.db')
    cursor = conn.cursor()

    # Delete the data created by Strategy Studio when you build your strategy. 
    # cursor.execute("DELETE FROM StrategyInstance WHERE instName = ?", (INSTANCE_NAME,))
    # cursor.execute("DELETE FROM StrategyType WHERE strategyTypeName = ?", (STRATEGY_NAME,))
    # print(f"Deleting instance {INSTANCE_NAME} of strategy {STRATEGY_NAME} from the sqlite database.")
    
    # # Alternatively, you could wipe the entire tables which contain the data created by Strategy Studio when you build your strategy.
    cursor.execute("DELETE FROM StrategyInstance")
    cursor.execute("DELETE FROM StrategyType")
    print(f"Deleting all instances of all strategies from the sqlite database.")
    # Display the contents of StrategyInstance & StrategyType tables
    print("Displaying the contents of StrategyInstance & StrategyType tables: (should be empty if deleted successfully)")
    cursor.execute("SELECT * FROM StrategyInstance")
    print(cursor.fetchall())
    cursor.execute("SELECT * FROM StrategyType")
    print(cursor.fetchall())

    conn.commit()
    conn.close()
    
else:
    print("Could not clean the sqlite database. New instance may not be created.")

