import argparse
import time
from datetime import datetime, timedelta
from dateutil import parser as date_parser
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import sys

from file_reader import get_file_info, read_lines
from log_filter import filter_by_level, filter_by_datetime
from log_analyzer import LogAnalyzer

class LogEventHandler(FileSystemEventHandler):
    def __init__(self, analyzer):
        self.analyzer = analyzer

    def on_modified(self, event):
        if event.src_path == self.analyzer.file_path:
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
    version = "1.0.2-Py"
    logo = f"""\033[0;31m


        ▄▄▌         ▄▄ • ▪   ▐ ▄ .▄▄ · ▪   ▄▄ •  ▄ .▄▄▄▄▄▄
        ██•  ▪     ▐█ ▀ ▪██ •█▌▐█▐█ ▀. ██ ▐█ ▀ ▪██▪▐█•██  
        ██▪   ▄█▀▄ ▄█ ▀█▄▐█·▐█▐▐▌▄▀▀▀█▄▐█·▄█ ▀█▄██▀▐█ ▐█.▪
        ▐█▌▐▌▐█▌.▐▌▐█▄▪▐█▐█▌██▐█▌▐█▄▪▐█▐█▌▐█▄▪▐███▌▐▀ ▐█▌·
        .▀▀▀  ▀█▄▀▪·▀▀▀▀ ▀▀▀▀▀ █▪ ▀▀▀▀ ▀▀▀·▀▀▀▀ ▀▀▀ · ▀▀▀ 


                           {version}
                    ✨ Created by Nighty3098

    """
    print(logo)

def main():
    if len(sys.argv) == 1 or '-h' in sys.argv or '--help' in sys.argv:
        print_logo()

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-i', '--input', required=True, help='Path to log file')
    parser.add_argument('-f', '--filter', help="""Comma separated log level""")
    parser.add_argument('-r', '--real-time', action='store_true', 
                        help='Enable real-time monitoring of log file changes')
    parser.add_argument('-d', '--date', help="""Date or date range (YYYY-MM-DD HH:MM)""")

    args = parser.parse_args()

    levels = args.filter.split(',') if args.filter else None

    if args.date:
        dates = [parse_date(d.strip()) for d in args.date.split(',')]
        date_range = (dates[0], dates[1]) if len(dates) == 2 else None
    else:
        date_range = None

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
    elif observer:
        observer.join()

if __name__ == "__main__":
    main()
