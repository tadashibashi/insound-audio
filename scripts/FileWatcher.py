from typing import Callable

try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
    has_watchdog = True
except ImportError:
    has_watchdog = False


class FileWatcher(FileSystemEventHandler):
    """
        Extremely simplistic file watcher that performs one callback on any
        change to the file tree
    """

    def __init__(self, callback: Callable):
        self.callback = callback

    def on_created(self, event):
        if not event.is_directory:
            print("New file created:", event.src_path)
            self.callback()

    def on_deleted(self, event):
        if not event.is_directory:
            print("File deleted:", event.src_path)
            self.callback()

    def on_moved(self, event):
        print("File moved:", event.src_path)
        self.callback()

    def on_modified(self, event):
        print("File modified:", event.src_path)
        self.callback()


def watch(callback: Callable):
    """
        Watch a file tree and perform callback on any file changes
    """
    observer = Observer()
    handler = FileWatcher(callback)
    observer.schedule(handler, path="./src/cpp", recursive=True)
    observer.start()

    try:
        while True:
            pass
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
