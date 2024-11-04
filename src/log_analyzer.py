import os
import platform
from datetime import datetime
from collections import defaultdict
from file_reader import get_file_info, read_lines
from log_filter import filter_by_level, filter_by_datetime
from colorizer import Colorizer

class LogAnalyzer:
    def __init__(self, file_path, levels=None, date_range=None):
        self.file_path = file_path
        self.levels = levels
        self.date_range = date_range
        self.lines_count = 0
        self.colorizer = Colorizer()
        self.level_counts = defaultdict(int)

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

    def output_statistics(self, lines):
        """Output statistics about the analyzed log."""
        self.lines_count = len(lines)
        self.count_levels(lines)

        for line in lines:
            level = self.extract_log_level(line)
            colored_line = self.colorizer.colorize(line.strip(), level)
            print(colored_line)

        print("\n\033[0;32m┌─────────────────────────────")
        for level, count in self.level_counts.items():
            if count > 0:
                print(f"│ {level.capitalize()}: {count}")
        print("└─────────────────────────────\033[0m")

        print("\n\033[0;34m┌─────────────────────────────")
        print(f"│ Lines read: {self.lines_count}")

        file_info = get_file_info(self.file_path)
        formatted_size = self.format_file_size(file_info['size'])
        print(f"│ File size: {formatted_size}")

        creation_date, modification_date = self.get_file_dates()
        if modification_date:
            print(f"│ Last modification date: {modification_date}")

        print("\033[0;34m└─────────────────────────────")

    def analyze(self):
        """Analyze the log file and output statistics."""
        lines = read_lines(self.file_path)

        if self.date_range:
            lines = filter_by_datetime(lines, self.date_range)

        if self.levels:
            lines = filter_by_level(lines, self.levels)

        self.output_statistics(lines)
