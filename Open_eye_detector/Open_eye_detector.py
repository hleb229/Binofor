import cv2
import time
from playsound import playsound

last_time = 0  # Глобальная переменная для отслеживания времени последнего звука.


def main():
    # Загрузка классификатора каскада Хаара для обнаружения глаз.
    eye_cascade = cv2.CascadeClassifier('haarcascade_eye.xml')

    def detect_eyes(img):
        global last_time  # Объявление глобальной переменной для доступа и изменения внутри функции.
        face_img = img.copy()  # Копирование изображения для обработки.
        # Обнаружение глаз на изображении.
        eyes_rect = eye_cascade.detectMultiScale(face_img, scaleFactor=1.15, minNeighbors=3)
        for (x, y, w, h) in eyes_rect:
            if w >= 10:  # Проверка размера глаз. Если глаза слишком малы, они игнорируются.
                # Проверка местоположения глаз. Если глаза не попадают в область отверстий для глаз, они игнорируются.
                if x >= 65 and y >= 245 and x <= 285 and y <= 415 or x >= 365 and y >= 245 and x <= 585 and y <= 415:
                    # Отрисовка прямоугольника вокруг глаз.
                    cv2.rectangle(face_img, (x, y), (x + w, y + h), (0, 0, 255), 5)
                    if eyes_rect.any():  # Проверка, что глаза были обнаружены.
                        # Добавление текста на изображение, указывающего на открытые глаза.
                        cv2.putText(face_img, "ЗАКРОЙ ГЛАЗА!", (30, 30), cv2.FONT_HERSHEY_COMPLEX, 1, (0, 0, 255), 2)
                        # Проверка времени с момента последнего воспроизведения звука.
                        if time.time() - last_time >= 1:
                            playsound("close-eyes.mp3", False) # Воспроизведение звука.
                            last_time = time.time() # Обновление времени последнего звука.

        return face_img  # Возвращение обработанного изображения.

    cap = cv2.VideoCapture(2)  # Инициализация захвата видеопотока с камеры.

    while True:
        ret, frame = cap.read(0)  # Захват кадра с камеры.
        frame = detect_eyes(frame)  # Обработка кадра.

        # Отображение обработанного кадра.
        cv2.imshow("Device camera", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):  # Ожидание нажатия клавиши "q" для выхода из цикла.
            break

    cap.release()  # Освобождение ресурсов камеры.
    cv2.destroyAllWindows()  # Закрытие всех окон OpenCV.


main()  # Вызов функции main().
