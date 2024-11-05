import os
import platform
import time
from datetime import datetime
from collections import defaultdict
from file_reader import get_file_info, read_lines
from log_filter import filter_by_level, filter_by_datetime
from colorizer import Colorizer
from prettytable import PrettyTable
from prettytable import MARKDOWN

class LogAnalyzer:
    def __init__(self, file_path, levels=None, date_range=None):
        self.file_path = file_path
        self.levels = levels
        self.date_range = date_range
        self.lines_count = 0
        self.colorizer = Colorizer()
        self.level_counts = defaultdict(int)
        self.isRealTime = False

    def format_file_size(self, size):
        """Format file size into appropriate units."""
        units = ['B', 'KB', 'MB', 'GB', 'TB', 'PB']
        for unit in units:
            if size < 1024:
                return f"{size:.2f} {unit}"
            size /= 1024
        return f"{size:.2f} PB"

    def get_file_dates(self):
        """Get the creation and modification dates of the file."""
        try:
            if platform.system() == 'Windows':
                import win32file, win32con
                handle = win32file.CreateFile(
                    self.file_path,
                    win32con.GENERIC_READ,
                    0,
                    None,
                    win32con.OPEN_EXISTING,
                    win32con.FILE_ATTRIBUTE_NORMAL,
                    None
                )
                creation_time = win32file.GetFileTime(handle)[0]
                win32file.CloseHandle(handle)
                creation_time = datetime.fromtimestamp(creation_time / 10000000 - 11644473600)
            else:
                creation_time = datetime.fromtimestamp(os.path.getctime(self.file_path))

            modification_time = datetime.fromtimestamp(os.path.getmtime(self.file_path))
            return (creation_time.strftime('%Y-%m-%d %H:%M:%S'),
                    modification_time.strftime('%Y-%m-%d %H:%M:%S'))
        except Exception as e:
            print(f"Error retrieving file dates: {e}")
            return None, None

    def extract_log_level(self, line):
        """Extracts the log level from a line."""
        levels = ["ERROR", "WARNING", "INFO", "DEBUG", "CRITICAL"]
        for level in levels:
            if level in line:
                return level
        return "INFO"

    def count_levels(self, lines):
        """Count occurrences of each log level in the lines."""
        for line in lines:
            level = self.extract_log_level(line)
            self.level_counts[level] += 1

    def output_statistics(self, lines, isRealTime):
        """Output statistics about the analyzed log."""
        self.lines_count = len(lines)
        self.count_levels(lines)

        for line in lines:
            level = self.extract_log_level(line)
            colored_line = self.colorizer.colorize(line.strip(), level)
            print(colored_line)

        if (isRealTime != True):
            level_table = PrettyTable()
            level_table.set_style(MARKDOWN)

            level_table.border = True
            level_table.header = False
            level_table.padding_width = 5
            level_table.vertical_char = "│"
            level_table.horizontal = "─"
            level_table._horizontal_align_char = ""
            level_table.junction_char = "│"

            level_table.field_names = ["Log Level", "Count"]
            for level, count in self.level_counts.items():
                if count > 0:
                    level_table.add_row([level.capitalize(), count])
            
            print("\n\n\033[0;32m│ ⬤  STATISTIC: \n│")
            print(f"\033[0;32m{level_table}\033[0m\n")

            file_table = PrettyTable()
            file_table.set_style(MARKDOWN)

            file_table.border = True
            file_table.header = False
            file_table.padding_width = 5
            file_table.vertical_char = "│"
            file_table.horizontal = "─"
            file_table._horizontal_align_char = ""
            file_table.junction_char = "│"

            file_table.field_names = ["Parameter", "Value"]
            file_table.add_row(["Lines read", self.lines_count])

            file_info = get_file_info(self.file_path)
            formatted_size = self.format_file_size(file_info['size'])
            file_table.add_row(["File size", formatted_size])

            creation_date, modification_date = self.get_file_dates()
            if modification_date:
                file_table.add_row(["Last modification date", modification_date])
            
            print("\033[0;34m│ ⬤  INFO: \n│")
            print(f"\033[0;34m{file_table}\033[0m\n")
        else:
            pass

    def analyze(self):
        """Analyze the log file and output statistics."""
        
        isRealTime = False
        lines = read_lines(self.file_path)

        if self.date_range:
            lines = filter_by_datetime(lines, self.date_range)

        if self.levels:
            lines = filter_by_level(lines, self.levels)

        self.output_statistics(lines, isRealTime)


    def analyze_real_time(self):
        """Analyze the log file in real-time and output only new lines."""
        
        previous_lines_count = len(read_lines(self.file_path))
        isRealTime = True

        while True:
            time.sleep(0.0000001)

            current_lines = read_lines(self.file_path)
            current_lines_count = len(current_lines)

            if current_lines_count > previous_lines_count:
                new_lines = current_lines[previous_lines_count:]

                if self.date_range:
                    new_lines = filter_by_datetime(new_lines, self.date_range)

                if self.levels:
                    new_lines = filter_by_level(new_lines, self.levels)

                self.output_statistics(new_lines, isRealTime)
                
                previous_lines_count = current_lines_count
