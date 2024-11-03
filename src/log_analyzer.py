import threading
import os
from datetime import datetime
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

    def format_file_size(self, size):
        """Format file size into appropriate units."""
        for unit in ['B', 'KB', 'MB', 'GB', 'TB', 'PB']:
            if size < 1024:
                return f"{size:.2f} {unit}"
            size /= 1024
        return f"{size:.2f} PB"

    def get_file_dates(self):
        """Get the creation and modification dates of the file."""
        try:
            file_stats = os.stat(self.file_path)
            creation_time = datetime.fromtimestamp(file_stats.st_ctime)
            modification_time = datetime.fromtimestamp(file_stats.st_mtime)

            creation_date = creation_time.strftime('%Y-%m-%d %H:%M:%S')
            modification_date = modification_time.strftime('%Y-%m-%d %H:%M:%S')

            return creation_date, modification_date
        except Exception as e:
            print(f"Error retrieving file dates: {e}")
            return None, None

    def extract_log_level(self, line):
        """Extracts the log level from a line. This is a placeholder implementation."""
        if "ERROR" in line:
            return "ERROR"
        elif "WARNING" in line:
            return "WARNING"
        elif "INFO" in line:
            return "INFO"
        elif "DEBUG" in line:
            return "DEBUG"
        return "INFO"

    def analyze(self):
        """Analyze the log file and output statistics."""
        lines = read_lines(self.file_path)

        if self.date_range:
            lines = filter_by_datetime(lines, self.date_range)

        if self.levels:
            lines = filter_by_level(lines, self.levels)

        self.lines_count = len(lines)

        for line in lines:
            level = self.extract_log_level(line)
            colored_line = self.colorizer.colorize(line.strip(), level)
            print(colored_line)

        print(f"\nNumber of lines: {self.lines_count}")

        file_info = get_file_info(self.file_path)

        formatted_size = self.format_file_size(file_info['size'])
        
        print(f"File size: {formatted_size}")

        creation_date, modification_date = self.get_file_dates()

        if creation_date and modification_date:
            print(f"Creation date: {creation_date}")
            print(f"Last modification date: {modification_date}")
