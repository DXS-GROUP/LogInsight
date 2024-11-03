import re
from datetime import datetime

def create_log_level_pattern(filter_level):
    """Creates a regex pattern for matching log levels in log lines."""
    filter_level_escaped = re.escape(filter_level)
    pattern = r'\|\s*' + filter_level_escaped + r'\s*\|'
    
    return pattern

def filter_by_level(lines, levels):
    """Filters rows by log level."""
    if not levels:
        return lines
    levels_set = set(level.lower() for level in levels)
    patterns = [create_log_level_pattern(level) for level in levels_set]
    combined_pattern = re.compile('|'.join(patterns), re.IGNORECASE)
    return [line for line in lines if combined_pattern.search(line)]


def filter_by_datetime(lines, date_range):
    """Filters rows by date and time."""
    filtered_lines = []
    for line in lines:
        match = re.match(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})', line)
        if match:
            log_date = datetime.strptime(match.group(1), '%Y-%m-%d %H:%M:%S')
            if date_range[0] <= log_date <= date_range[1]:
                filtered_lines.append(line)
    return filtered_lines
