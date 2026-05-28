import serial
import serial.tools.list_ports
from vpython import *

def get_stm32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'usb' in port.device.lower() or 'serial' in port.device.lower():
            return port.device
    return None
# 포트 연결
target_port = get_stm32_port()
if target_port:
    ser = serial.Serial(target_port, 115200, timeout=1)
    print(f"🚀 Connected to {target_port}")
else:
    print("❌ STM32 Not Found")
    exit()

# 3D 화면 설정
scene = canvas(title='STM32 IMU Real-time 3D', width=800, height=600)
bridge = box(length=6, height=0.2, width=4, color=color.cyan)
label_info = label(pos=vector(0, 4, 0), text='Roll: 0, Pitch: 0', height=16)

# 축 표시 가이드
x_axis = arrow(axis=vector(2, 0, 0), color=color.red, shaftwidth=0.05)
y_axis = arrow(axis=vector(0, 2, 0), color=color.green, shaftwidth=0.05)

while True:
    if ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8').strip()
            data = line.split(',')

            if len(data) == 2:
                # 라디안 변환
                roll_val = radians(float(data[0]))
                pitch_val = radians(float(data[1]))

                # 3D 모델의 방향을 Quaternion(쿼터니언) 기반 회전으로 직접 설정
                # 이렇게 하면 물체가 꼬이지 않고 정확한 각도를 유지합니다.
                bridge.up = vector(0, 1, 0)
                bridge.axis = vector(1, 0, 0)
                bridge.rotate(angle=pitch_val, axis=vector(0, 0, 1))
                bridge.rotate(angle=roll_val, axis=vector(1, 0, 0))

                label_info.text = f'Roll: {data[0]}, Pitch: {data[1]}'
        except:
            continue

    rate(100)  # 시각화 갱신 속도