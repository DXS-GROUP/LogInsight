import re
from datetime import datetime

def filter_by_level(lines, levels):
    """Filters rows by log level."""
    if not levels:
        return lines
    levels_set = set(levels)
    return [line for line in lines if any(level in line for level in levels_set)]

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
