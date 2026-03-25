import sys
import socket
from PySide2.QtWidgets import QApplication, QMainWindow, QGridLayout, QLabel, QLineEdit, QPushButton, QTextEdit, QWidget
from PySide2.QtCore import QThread, Signal

class UDPReceiver(QThread):
    data_received = Signal(bytes)

    def __init__(self, host='0.0.0.0', port=1600):
        super().__init__()
        self.host = host
        self.port = port
        self.running = True

    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((self.host, self.port))
        while self.running:
            try:
                data, addr = sock.recvfrom(65535)  # max UDP size
                if data:
                    self.data_received.emit(data)
            except Exception as e:
                break
        sock.close()

    def stop(self):
        self.running = False
        self.wait()

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('UDP Payload Monitor')
        self.setGeometry(300, 300, 600, 400)

        self.total_bytes = 0

        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        layout = QGridLayout()

        self.status_label = QLabel('Listening on UDP port 1600')
        self.payload_label = QLabel('Latest Payload Size: 0 Bytes')
        self.total_mb_label = QLabel('Total Received: 0.00 MB')
        self.total_gb_label = QLabel('Total Received: 0.00 GB')

        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)

        layout.addWidget(self.status_label, 0, 0, 1, 2)
        layout.addWidget(self.payload_label, 1, 0)
        layout.addWidget(self.total_mb_label, 1, 1)
        layout.addWidget(self.total_gb_label, 2, 1)
        layout.addWidget(self.log_text, 3, 0, 1, 2)

        central_widget.setLayout(layout)

        self.udp_receiver = UDPReceiver(port=1600)
        self.udp_receiver.data_received.connect(self.handle_data)
        self.udp_receiver.start()

    def handle_data(self, data):
        size_bytes = len(data)
        self.total_bytes += size_bytes

        mb = self.total_bytes / (1024*1024)
        gb = self.total_bytes / (1024*1024*1024)

        self.payload_label.setText(f'Latest Payload Size: {size_bytes} Bytes')
        self.total_mb_label.setText(f'Total Received: {mb:.2f} MB')
        self.total_gb_label.setText(f'Total Received: {gb:.2f} GB')

        self.log_text.append(f'Received {size_bytes} bytes')

    def closeEvent(self, event):
        self.udp_receiver.stop()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
