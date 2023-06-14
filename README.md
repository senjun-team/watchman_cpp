# Watchman Cpp
Сервис для запуска задач и тестов. Оркестратор контейнеров, в которых выполняется код от пользователей.

## Сборка проекта
Для сборки прокета необходимо:

— установить в систему библиотеки: fmt, boost 1.66 или выше, curl;

— обновить самбодуль с DockerClient: 
```bash
git submodule update --init --recursive
```

В проект исходниками скопированы restinio, так как не планируем, чтобы они менялись.

## API
У сервиса единственная апишка `/check`:
```
curl -X 'POST' \
  'http://127.0.0.1:8000/check' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "container_type": "python",
  "source_test": "exit(0)",
  "source_run": "print(123)"
}'
```

Ответ:
```
{"error_code":0,"output":"123","tests_error_code":0,"tests_error":""}
```