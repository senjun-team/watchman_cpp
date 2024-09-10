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

## Тестирование Docker API

https://docs.docker.com/reference/api/engine/v1.41/#tag/Container/operation/ContainerList

```shell
curl  --unix-socket /var/run/docker.sock http://v1.41/containers/json
```


https://docs.docker.com/reference/api/engine/v1.41/#tag/Container/operation/PutContainerArchive

```shell
curl -X 'PUT' \
  --unix-socket /var/run/docker.sock http:/v1.41/containers/273938a0703ac4584d6c60b47f28c4034bf4f12ede2ed72f7b4a93812af1213c/archive?path=/home/code_runner/ \
  -H 'accept: application/json' \
  -H 'Content-Type: application/x-tar' \
  -T archive.tar
```