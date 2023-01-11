# Watchman Cpp



## Сборка проекта

Для сборки прокета необходимо:

— установить в систему библиотеки: fmt, boost 1.66 или выше, curl;

— обновить самбодуль с DockerClient: 
```bash
git submodule update --init --recursive
```

В проекте исходниками скопированы restinio и unifex, так как не планируем, 
чтобы они менялись.

