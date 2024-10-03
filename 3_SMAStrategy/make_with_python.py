import subprocess
import sys
import time


def make_with_python(command, timeout=100):
    try:
        # Execute the command with check=True to automatically raise an exception for non-zero return codes
        subprocess.run(command, check=True, timeout=timeout)
        print("Command executed successfully.")
        return True  # Indicates success
    except:
        print("An error occurred.", sys.exc_info())
        return False


if __name__ == "__main__":
    # Command to execute
    episode_parameters = "aggressiveness=0.1|position_size=100|debug=false|short_window_size=10|long_window_size=30"
    start_date = "2022-01-01"
    end_date = "2022-01-29"
    if sys.argv[1] == "0":
        commands = [
            ["make", "start_server"],
            ["make", "create_instance"],
            ["make", "run_backtest"],
        ]
    else:
        commands = [
            ["make", "start_server"],
            ["make", "create_instance"],
            ["make", "edit_params", f"EPISODE_PARAMETERS={episode_parameters}"],
            [
                "make",
                "run_backtest_with_results",
                f"START_DATE={start_date}",
                f"END_DATE={end_date}",
            ],
        ]

    # Execute the commands, stopping if one fails
    for command in commands:
        success = make_with_python(command, int(sys.argv[2]))
        if not success:
            sys.exit(1)  # Terminate the program if a command fails
        time.sleep(5)
