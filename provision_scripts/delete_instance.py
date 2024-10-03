import sqlite3
import sys
import os

os.chdir(os.path.expanduser('~/ss/bt/db'))
print(f"Python script at: {os.getcwd()}")
if len(sys.argv) > 2 and os.path.exists('strategy.db'):
    InstanceName = sys.argv[1]
    StrategyName = sys.argv[2]
    
    conn = sqlite3.connect('strategy.db')
    cursor = conn.cursor()

    # Delete the data created by Strategy Studio when you build your strategy. 
    # cursor.execute("DELETE FROM StrategyInstance WHERE instName = 'InstanceName1'")
    # cursor.execute("DELETE FROM StrategyType WHERE strategyTypeName = 'Helloworld_Strategy'")
    # cursor.execute("DELETE FROM StrategyInstance WHERE instName = ?", (InstanceName,))
    # cursor.execute("DELETE FROM StrategyType WHERE strategyTypeName = ?", (StrategyName,))
    # print(f"Deleting instance {InstanceName} of strategy {StrategyName} from the sqlite database.")
    
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
