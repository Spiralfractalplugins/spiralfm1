# SPIRALFM1 — запуск веб-установщика

Прошивка ставится из браузера: страница отдаёт пакет `.fwsc`, говорит с FM-1 по **Web MIDI c SysEx**
(`requestMIDIAccess({ sysex: true })`), держит устройство в boot-режиме и пишет загрузчик вместе с
приложением. Тот же сайт работает и с твоей локальной сборки — ничего на GitHub публиковать не
обязательно.

Нужно: **Chrome или Edge** (Web MIDI с SysEx в Safari и Firefox не поддержан), USB-кабель с данными,
без хаба, и чтобы ни одно другое приложение или вкладка не держали MIDI — иначе страница пишет
`FM-1 not found`. На FM-1 при нажатии **Install** держи **POWER → B1**. Перед установкой сделай
резервную копию в [редакторе](#редактор) — Projects → Backup.

## Локально

Из корня репозитория (`build/felucca.fwsc` уже собран, или собери его через `./build.sh`):

```sh
python3 web/make_site.py build/felucca.fwsc dev /tmp/spiralfm1-site
cd /tmp/spiralfm1-site && python3 -m http.server 8000
```

Затем в Chrome:

- установщик — `http://localhost:8000/webapp/installer/`
- редактор — `http://localhost:8000/webapp/editor/`

Три аргумента `make_site.py`: пакет `.fwsc`, номер версии (попадёт в название файла и на страницу),
куда положить сайт. Флаг `--beta` добавляет сверху установщика баннер бета-канала — текст берётся из
`web/beta_banner.html`, если файла там нет, подставляется короткая заглушка.

Что делает скрипт:

```
site/index.html                    редирект на /webapp/installer/
site/webapp/installer/index.html   web/index_pkg.html + встроенные fm1pkg.js, fm1ota.js и метаданные
site/webapp/editor/index.html      web/editor.html (+ fukiai.ttf, FUKIAI-LICENSE.txt)
site/firmware/spiralfm1-ВЕР.fwsc   сам пакет
```

Идентичность устройства (`FM-1_9xx`) и SHA-256 пакета скрипт читает из самого `.fwsc` и зашивает в
страницу: установщик сверит скачанное с хешем, а устройство после записи назовёт свою семью. Сборка
остановится, если в пакете нет метки `FELUCCA-LOADER-1` или идентичность не `FM-1_9xx` — это значит,
что `.fwsc` не от SPIRALFM1.

Важные мелочи:

- Открывай **`localhost`**, а не `127.0.0.1` и не IP-адрес: Web MIDI доступен только в secure
  context, а `localhost` — исключение из правил. Для любого другого хоста нужен HTTPS (`mkcert` или
  GitHub Pages).
- Если страница отвечает `MIDI (SysEx) access was not allowed` — разреши MIDI для этого сайта в
  настройках адреса в адресной строке Chrome и нажми Install снова.
- Сервер должен отдавать **весь** каталог `site/`: путь к пакету на странице относительный
  (`../../firmware/…`), поэтому `python3 -m http.server` внутри `site/webapp/installer/` не найдёт
  `.fwsc`.
- Windows: `tools/windows/INSTALL-SPIRALFM1.bat` собирает в WSL и сам поднимает сайт на
  `http://localhost:8766/webapp/installer/` (редактор — `tools/windows/OPEN-EDITOR.bat`).

## Редактор

`http://localhost:8000/webapp/editor/` (или `https://spiralfractalplugins.github.io/spiralfm1/webapp/editor/`):
**Connect**, и редактор следует за устройством вживую — крутишь ручку на FM-1, в редакторе едет.
Сообщения SysEx и формат бэкапа — [web/EDITOR_PROTOCOL.md](EDITOR_PROTOCOL.md). Если после Connect
пишет `SPIRALFM1 not found` — Chrome не дал доступ к MIDI (разреши в настройках сайта в адресной
строке), устройство не подключено, или MIDI-порт уже занят другим приложением или вкладкой.

## На GitHub Pages

Ссылки из README (`https://spiralfractalplugins.github.io/spiralfm1/...`) отдаёт
`.github/workflows/pages.yml`: он собирает прошивку тем же `./build.sh`, прогоняет `make_site.py` и
публикует каталог `site/` как артефакт Pages.

Одноразовая настройка репозитория (без неё публикация невозможна, и API отвечает `404`):

```sh
gh api -X POST repos/Spiralfractalplugins/spiralfm1/pages -f build_type=workflow
```

Это же Settings → Pages → Build and deployment → **Source: GitHub Actions**. Дальше сайт обновляется
сам на каждый push в `main`, вручную — `gh workflow run pages`. Проверить, что выложилось:

```sh
gh api repos/Spiralfractalplugins/spiralfm1/pages -q '.status + " " + .html_url'   # built_type=workflow
curl -s -o /dev/null -w '%{http_code}\n' https://spiralfractalplugins.github.io/spiralfm1/webapp/installer/
curl -s -o /dev/null -w '%{http_code} %{size_download}\n' https://spiralfractalplugins.github.io/spiralfm1/firmware/spiralfm1-1.0.fwsc
```

Два сбоя, которые легко принять за что-то другое:

- `FileNotFoundError: 'rsvg-convert'` в шаге Build — на раннере нет `librsvg2-bin`
  (`tools/gen_spiralfm1_logo.py` конвертит SVG им через `subprocess`). Чинится установкой пакета в
  workflow, а не кодом генератора.
- `pages` зелёный, но сайт пустой — Source не переключён на GitHub Actions, или в `pages.yml` нет
  прав `pages: write` / `id-token: write` и step'а `actions/deploy-pages@v4`.

`release.yml` на теге `vX.Y` собирает тот же сайт и кладёт его в релиз как
`spiralfm1-ВЕР-site.zip` — архив распадовывается локально точно так же, как `site/` выше.
