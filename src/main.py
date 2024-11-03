import argparse
import time
from datetime import datetime, timedelta
from dateutil import parser as date_parser
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

from file_reader import get_file_info, read_lines
from log_filter import filter_by_level, filter_by_datetime
from log_analyzer import LogAnalyzer


class LogEventHandler(FileSystemEventHandler):
    def __init__(self, analyzer):
        self.analyzer = analyzer

    def on_modified(self, event):
        if event.src_path == self.analyzer.file_path:
            # print("File modified. Updating the output...")
            self.analyzer.analyze()


def parse_date(date_str):
    """Parses a date or time string into a datetime object."""
    try:
        return date_parser.parse(date_str)
    except ValueError:
        print(f"Error: '{date_str}' is not a valid date or time.")
        return None


def print_logo():
    """Prints the logo of the program."""
    logo = """\033[0;31m


        ▄▄▌         ▄▄ • ▪   ▐ ▄ .▄▄ · ▪   ▄▄ •  ▄ .▄▄▄▄▄▄
        ██•  ▪     ▐█ ▀ ▪██ •█▌▐█▐█ ▀. ██ ▐█ ▀ ▪██▪▐█•██  
        ██▪   ▄█▀▄ ▄█ ▀█▄▐█·▐█▐▐▌▄▀▀▀█▄▐█·▄█ ▀█▄██▀▐█ ▐█.▪
        ▐█▌▐▌▐█▌.▐▌▐█▄▪▐█▐█▌██▐█▌▐█▄▪▐█▐█▌▐█▄▪▐███▌▐▀ ▐█▌·
        .▀▀▀  ▀█▄▀▪·▀▀▀▀ ▀▀▀▀▀ █▪ ▀▀▀▀ ▀▀▀·▀▀▀▀ ▀▀▀ · ▀▀▀ 

                    ✨ Created by Nighty3098

    """
    print(logo)

def main():
    print_logo()

    parser = argparse.ArgumentParser(description="\033[0;32m🐍 Log analyzer\033[0;33m")
    parser.add_argument('-i', '--input', required=True, help='Path to log file')
    parser.add_argument('-f', '--filter', help="""Comma separated log level. Example: \033[0;32m -f ERROR,WARNING\033[0;33m""")
    parser.add_argument('-r', '--real-time', action='store_true', help='Real-time')
    parser.add_argument('-d', '--date', help="""Date or date range (YYYY-MM-DD or HH:MM).
                                            Example:
                                            \033[0;32m -d "2024-07-31 23:42:42.960, 2024-08-09 12:42:34.947"\033[0;33m
                                            or \033[0;32m -d "2024-07-31 23:42:42"
                                            """)
    args = parser.parse_args()

    levels = args.filter.split(',') if args.filter else None
    
    date_range = None
    if args.date:
        dates = args.date.split(',')
        parsed_dates = []
        
        for date_str in dates:
            parsed_date = parse_date(date_str.strip())
            if parsed_date:
                parsed_dates.append(parsed_date)

        if len(parsed_dates) == 1:
            single_date = parsed_dates[0]
            date_range = (single_date, single_date + timedelta(days=1))
        elif len(parsed_dates) == 2:
            date_range = (parsed_dates[0], parsed_dates[1])

    from log_analyzer import LogAnalyzer  
    analyzer = LogAnalyzer(args.input, levels=levels, date_range=date_range)

    observer = None

    if args.real_time:
        event_handler = LogEventHandler(analyzer)
        observer = Observer()
        observer.schedule(event_handler, path=args.input, recursive=False)
        observer.start()
    
    analyzer.analyze()

    if args.real_time:
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            observer.stop()
    
    if observer:
        observer.join()


if __name__ == "__main__":
    main()
