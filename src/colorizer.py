# colorizer.py

class Colorizer:
    """ANSI escape codes for colors"""
    COLORS = {
        'INFO': '\033[0;32m',    # Green
        'WARNING': '\033[0;33m', # Yellow
        'ERROR': '\033[0;31m',   # Red
        'DEBUG': '\033[0;34m',   # Blue
        'TRACE': '\033[0m', # WHITE
        'RESET': '\033[0m'     # Reset to default color
    }

    def __init__(self):
        pass

    def colorize(self, message, level):
        """Colorizes the log message based on its level."""
        color = self.COLORS.get(level.upper(), self.COLORS['RESET'])
        return f"{color}{message}{self.COLORS['RESET']}"
