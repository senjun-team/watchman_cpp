# Сборка rpm
Здесь описаны шаги, выполненные для сборки rpm в контейнере.

### Билдим образ контейнера
В файле accomodation/build_rpm/**Dockerfile** установите номер релиза.
Этот номер будет в результирующей rpm. Например, watchman_cpp-**0.0.13**-1.el9.x86_64.rpm.
После этого можно билдить образ контейнера:
```bash
cd accomodation/build_rpm
sh accomodation/build_rpm/docker_build_image.sh
```

### Запускаем сборку rpm
```bash
sh accomodation/build_rpm/docker_run.sh
```
В консоли будет происходить процесс сборки. В конце сборки будут строчки:
```bash
+ RPM_EC=0
++ jobs -p
+ exit 0
```
После этого в папке accomodation/build_rpm появится папка **RPMS** с артефактами сборки.

В `docker_run.sh` при монтировании тома используется флаг `z` для SELinux. Он чинит ошибку доступа при обращении из контейнера к `/mnt/watchman_cpp/accomodation/build_rpm/build_rpm.sh`. Если вы не используете механизм SELinux, можете этот флаг убрать. 