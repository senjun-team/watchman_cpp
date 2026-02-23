# Watchman Cpp
Сервис для запуска задач и тестов. Оркестратор Docker-контейнеров, в которых выполняется код от пользователей.

## Сборка проекта
Для сборки прокета необходимо:

— установить в систему библиотеки: boost 1.66 или выше, curl;

— обновить самбодули: 
```shell
git submodule update --init --recursive
```

Для сборки выполните:

```shell
cmake -B build/
cmake --build build/
```

## Запуск

Перед запуском склонируйте к себе [проект с нашими Docker-образами.](https://github.com/senjun-team/senjun-images) Выполните в нем скрипт, который собирает все образы кроме самого тяжелого:

```shell
sh create_images.sh
```

После того как образы собраны, можно запускать сервис. Он лежит в `build/bin`:

```shell
./watchman_cpp
```

Рядом с ним лежит конфиг `watchman_cpp_config.json`. Чтобы поменять количество контейнеров, вносите в него изменения и перезапускайте сервис.

Если все в порядке, при запуске сервиса вы увидите примерно такой лог:

```
...
[2025-11-16 13:57:06.058] [sync_logger] [info] Service launched
[2025-11-16 13:57:06.059] [sync_logger] [info] Watchman working on 8000 port
```

Теперь можно пробовать его курлить.

## API
У сервиса единственная апишка `/check`:
```shell
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
```json
{"error_code":0,"output":"123","tests_error_code":0,"tests_error":""}
```

## Тестирование Docker API

Нужно на случай, если в вашей системе что-то не так с самим докером.

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