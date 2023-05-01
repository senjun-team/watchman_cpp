# Сборка rpm
Здесь описаны шаги, выполненные для сборки rpm в контейнере.

## Работа с докером

### Билдим образ контейнера
Из директории watchman_cpp/accomodation запускаем команду
```bash
docker build -t watchman_rpm .
```

### Запускаем контейнер
```bash
docker run -itd watchman_rpm
```

### Заходим в контейнер
```bash
docker exec -it container_id bash
```

Все остальные действия происходят внутри контейнера

## Создаём дерево папок
```bash
rpmdev-setuptree
```
После этой команды в директории /home будет создана папка **rpmbuild**, 
внутри которой будет несколько папок, необходимых для rpm. 

### SOURCES
Сюда нужно скачать архвив **.tar.gz** с нужными исходниками.
Архив автоматически формируется, когда создается Release на gitlab-e. Получить архив можно через wget:
```bash
wget --header='PRIVATE-TOKEN: ACCESS_TOKEN' https://gitlab.com/senjun/watchman_cpp/-/archive/0.0.13/watchman_cpp-0.0.13.tar.gz
```
где: **ACCESS_TOKEN** формируем самостоятельно в настройках gitlab/preferences/access tokens.
Полученный архив распаковывать не нужно.

### SPECS
Сюда необходимо положить .spec-файл (спека) для сборки проекта. Для watchman_cpp 
она лежит в

**watchman_cpp/accomodation/watchman_cpp-el9.spec**

Нужно удостовериться, что тег скачанного архива и версия внутри спеки совпадают.
Если это не так, то руками меняем значение **Version** внутри спеки:
```bash
vim watchman_cpp-el9.spec
```

#### Сборка rpm
Для запуска сборки rpm внутри директории **SPECS** выполняем:
```bash
rpmbuild -bb watchman_cpp-el9.spec
```
Ждём, пока соберется rpm. Результирующий rpm-пакет будет положен в папку **RPMS**

## Получение rpm host-системе
Необходимо скопировать собранную rpm в host-систему, чтобы отдать её на прод.
```bash
docker cp contaner_id:/root/rpmbuild/RPMS/x86_64/watchman_cpp-0.0.13-1.el9.x86_64.rpm ~/Shipilov/SenJun/rpms
```
