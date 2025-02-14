# tym

[![CircleCI](https://circleci.com/gh/endaaman/tym.svg?style=svg)](https://circleci.com/gh/endaaman/tym) [![Gitter chat](https://badges.gitter.im/tym-terminal/gitter.png)](https://gitter.im/tym-terminal/Lobby)

`tym` is a Lua-configurable terminal emulator base on VTE.

## Installation

### Arch Linux

```
$ yay -S tym
```

### Other distros

Download the latest release from [Releases](https://github.com/endaaman/tym/releases), extract it and run as below

```
$ ./configure
$ sudo make install
```

<details><summary>Build dependencies (click to open)</summary>
<p>

#### Arch Linux

```
$ sudo pacman -S vte3 lua53
```

#### Ubuntu

```
$ sudo apt install libgtk-3-dev libvte-2.91-dev liblua5.3-dev libpcre2-dev
```

#### Void Linux
```
$ sudo xbps-install -S vte3-devel lua-devel
```

#### Other distros / macOS / Windows

We did not check which packages are needed to build on other distros or OS. We are waiting for your contribution ;)

</p>
</details>

## Configuration

If `$XDG_CONFIG_HOME/tym/config.lua` exists, it is executed when the app starts. You can change the path by `--use`/`-u` option.

```lua
-- At first, you need to require tym module
local tym = require('tym')

-- set individually
tym.set('width', 100)

tym.set('font', 'DejaVu Sans Mono 11')

-- set by table
tym.set_config({
  shell = '/usr/bin/fish',
  cursor_shape = 'underline',
  autohide = true,
  color_foreground = 'red',
})
```

All available config values are shown below.

| field name | type | default value | description |
| --- | --- | --- | --- |
| `shell` | string | `$SHELL` → `vte_get_user_shell()` → `'/bin/sh'` | Shell to execute. |
| `term` | string | `'xterm-256color'` | Value of `$TERM`. |
| `title` | string | `'tym'` | Initial window title. |
| `font` | string | `''` | You can specify font with `'FAMILY-LIST [SIZE]'`, for example `'Ubuntu Mono 12'`. The value is parsed by [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If empty string is set, the system default fixed width font will be used. |
| `icon` | string | `'utilities-terminal'` | Name of icon. cf. [Icon Naming Specification](https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html) |
| `role` | string | `''` | Unique identifier for the window. If empty string is set, no value set. (cf. [gtk_window_set_role()](https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-set-role)) |
| `cursor_shape` | string | `'block'` | `'block'`, `'ibeam'` or `'underline'` can be used. |
| `cursor_blink_mode` | string | `'system'` | `'system'`, `'on'` or `'off'` can be used. |
| `cjk_width` | string | `'narrow'` | `'narrow'` or `'wide'` can be used. |
| `background_image` | string | `''` | Path to background image file. |
| `uri_schemes` | string | `'http https file mailto'` | Space-separated list of URI schemes to be highlighted and clickable. Specify empty string to disable highlighting. Specify `'*'` to accept any strings valid as schemes (according to RFC 3986). |
| `width` | integer | `80` | Initial columns. |
| `height` | integer | `22` | Initial rows. |
| `scale` | integer | `100` | Font scale in **percent(%)** |
| `padding_horizontal`  | integer | `0` | Horizontal padding. |
| `padding_vertical`  | integer | `0` | Vertical padding. |
| `scrollback_length` | integer | `512` | Length of the scrollback buffer. |
| `ignore_default_keymap` | boolean | `false` | Whether to use default keymap. |
| `autohide` | boolean | `false` | Whether to hide mouse cursor when the user presses a key. |
| `rewrap` | boolean | `false` | Rewrap the content when terminal size changed. |
| `silent` | boolean | `false` | Whether to beep when bell sequence is sent. |
| `isolated` | boolean | `false` | If true, the app instance will be insolated from D-Bus and no longer have ability to handle D-Bus signals/method calls. |
| `color_window_background` | string | `''` | Color of the terminal window. It is seen when `'padding_horizontal'` `'padding_vertical'` is not `0`. If you set `'NONE'`, the window background will not be drawn. |
| `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground`, `color_bold`, `color_0` ... `color_15` | string | [See next section](#user-content-theme-customization) | You can specify standard color string such as `'#f00'`, `'#ff0000'`, `'rgba(22, 24, 33, 0.7)'` or `'red'`. It will be parsed by [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If empty string is set, the VTE default color will be used. If you set `'NONE'` for `color_background`, the terminal background will not be drawn.|


## Theme customization

When `$XDG_CONFIG_HOME/tym/theme.lua` exists, it is loaded **before** loading config. You can change the path by `--theme`/`-t` option. The following is an example, whose color values are built-in default. They were ported from [iceberg](https://cocopon.github.io/iceberg.vim/).

```lua
local bg = '#161821'
local fg = '#c6c8d1'
return {
  color_background = bg,
  color_foreground = fg,
  color_bold = fg,
  color_cursor = fg,
  color_cursor_foreground = bg,
  color_highlight = fg,
  color_highlight_foreground = bg,
  color_0  = bg,
  color_1  = '#e27878',
  color_2  = '#b4be82',
  color_3  = '#e2a478',
  color_4  = '#84a0c6',
  color_5  = '#a093c7',
  color_6  = '#89b8c2',
  color_7  = fg,
  color_8  = '#6b7089',
  color_9  = '#e98989',
  color_10 = '#c0ca8e',
  color_11 = '#e9b189',
  color_12 = '#91acd1',
  color_13 = '#ada0d3',
  color_14 = '#95c4ce',
  color_15 = '#d2d4de',
}
```

You need to return the color map as table.

<details><summary>Color correspondence (click to open)</summary>
<div>

```
color_0  : black (background)
color_1  : red
color_2  : green
color_3  : brown
color_4  : blue
color_5  : purple
color_6  : cyan
color_7  : light gray (foreground)
color_8  : gray
color_9  : light red
color_10 : light green
color_11 : yellow
color_12 : light blue
color_13 : pink
color_14 : light cyan
color_15 : white
```

</div>
</details>


## Keymap

### Default keymap

| Key             | Action                       |
| :-------------- | :--------------------------- |
| Ctrl Shift c    | Copy selection to clipboard. |
| Ctrl Shift v    | Paste from clipboard.        |
| Ctrl Shift r    | Reload config file.          |

### Customizing keymap

You can register keymap(s) using `tym.set_keymap(accelerator, func)` or `tym.set_keymaps(table)`. `accelerator` must be in a format parsable by [gtk_accelerator_parse()](https://developer.gnome.org/gtk3/stable/gtk3-Keyboard-Accelerators.html#gtk-accelerator-parse). If turethy value is returned, the event propagation will **not be stopped**.

```lua
-- also can set keymap
tym.set_keymap('<Ctrl><Shift>o', function()
  local h = tym.get('height')
  tym.set('height', h + 1)
  tym.notify('Set window height :' .. h)
end)

-- set by table
tym.set_keymaps({
  ['<Ctrl><Shift>t'] = function()
    tym.reload()
    tym.notify('reload config')
  end,
  ['<Ctrl><Shift>v'] = function()
    -- reload and notify
    tym.send_key('<Ctrl><Shift>t')
  end,

  ['<Shift>y'] = function()
    tym.notify('Y has been pressed')
    return true -- notification is shown and `Y` will be inserted
  end,
  ['<Shift>w'] = function()
    tym.notify('W has been pressed')
    -- notification is shown but `W` is not inserted
  end,
})
```

## Lua API

| Name                                 | Return value | Description |
| ------------------------------------ | ------------ | ----------- |
| `tym.get(key)`                       | any      | Get config value. |
| `tym.set(key, value)`                | void     | Set config value. |
| `tym.get_default_value(key)`         | any      | Get default config value. |
| `tym.get_config()`                   | table    | Get whole config. |
| `tym.set_config(table)`              | void     | Set config by table. |
| `tym.reset_config()`                 | void     | Reset all config. |
| `tym.set_keymap(accelerator, func)`  | void     | Set keymap. |
| `tym.unset_keymap(accelerator)`      | void     | Unset keymap. |
| `tym.set_keymaps(table)`             | void     | Set keymaps by table. |
| `tym.reset_keymaps()`                | void     | Reset all keymaps. |
| `tym.set_hook(hook_name, func)`      | void     | Set a hook. |
| `tym.set_hooks(table)`               | void     | Set hooks. |
| `tym.reload()`                       | void     | Reload config file.|
| `tym.reload_theme()`                 | void     | Reload theme file. |
| `tym.send_key()`                     | void     | Send key press event. |
| `tym.set_timeout(func, interval=0)`  | int(tag) | Set timeout. return true in func to execute again. |
| `tym.clear_timeout(tag)`             | void     | Clear the timeout. |
| `tym.put(text)`                      | void     | Feed text. |
| `tym.bell()`                         | void     | Sound bell. |
| `tym.open(uri)`                      | void     | Open URI via your system default app like `xdg-open(1)`. |
| `tym.notify(message, title='tym')`   | void     | Show desktop notification. |
| `tym.copy(text, target='clipboard')` | void     | Copy text to clipboard. As `target`, `'clipboard'`, `'primary'` or `secondary` can be used. |
| `tym.copy_selection(target='clipboard')` | void | Copy current selection. |
| `tym.paste(target='clipboard')`      | void     | Paste clipboard. |
| `tym.check_mod_state(accelerator)`   | bool     | Check if the mod key(such as `'<Ctrl>'` or `<Shift>`) is being pressed. |
| `tym.color_to_rgba(color)`           | r, g, b, a | Convert color string to RGB bytes and alpha float using [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). |
| `tym.rgba_to_color(r, g, b, a)`      | string   | Convert RGB bytes and alpha float to color string like `rgba(255, 128, 0, 0.5)` can be used in color option such as `color_background`. |
| `tym.rgb_to_hex(r, g, b)`            | string   | Convert RGB bytes to 24bit HEX like `#ABCDEF`. |
| `tym.hex_to_rgb(hex)`                | r, g, b  | Convert 24bit HEX like `#ABCDEF` to RGB bytes. |
| `tym.get_monitor_model()`            | string   | Get monitor model on which the window is shown. |
| `tym.get_cursor_position()`          | int, int | Get where column and row the cursor is. |
| `tym.get_clipboard(target='clipboard')` | string | Get content in the clipboard. |
| `tym.get_selection()`                | string   | Get selected text. |
| `tym.get_text(start_row, start_col, end_row, end_col)` | string | Get text on the terminal screen. If you set `-1` to `end_row` and `end_col`, the target area will be the size of termianl. |
| `tym.get_config_path()`              | string   | Get full path to config file. |
| `tym.get_theme_path()`               | string   | Get full path to theme file. |
| `tym.get_pid()`                      | integer  | Get pid. |
| `tym.get_version()`                  | string   | Get version string. |

### Hooks

| Name | Param | Default action | Description |
| --- | --- | --- | --- |
| `title`       | title  | changes title | If string is returned, it will be used as the new title. |
| `bell`        | nil    | makes the window urgent when it is inactive. | If true is returned, the window will not be urgent. |
| `clicked`     | button, uri | If URI exists under cursor, opens it | Triggered when mouse button is pressed. |
| `scroll`      | delta_x, delta_x, mouse_x, mouse_y  | scroll buffer | Triggered when mouse wheel is scrolled. |
| `drag`        | filepath  | feed filepath to the console | Triggered when files are dragged to the screen. |
| `activated`   | nil    | nothing | Triggered when the window is activated. |
| `deactivated` | nil    | nothing | Triggered when the window is deactivated. |
| `selected`    | string | nothing | Triggered when the text in the terminal screen is selected. |
| `unselected`  | nil    | nothing | Triggered when the selection is unselected. |
| `signal`      | string | nothing | Triggered when `me.endaaman.tym.hook` signal is received. |

If turethy value is returned in a callback function, the default action is will **be canceled**.

```lua
tym.set_hooks({
  title = function(t)
    tym.set('title', 'tym - ' .. t)
    return true -- this is needed to cancenl default title application
  end,
})

tym.set_hook('clicked', function(button, uri)
  print('you pressed button:', button) -- 1:left, 2:middle, 3:right...
  if uri then
    print('you clicked URI: ', uri)
    if button == 2 then
      -- disable URI open when middle button is clicked
      return true
    end
  end
end)
```

## Interprocess communication using D-Bus

Each tym window has an unique ID, which can be checked by `tym.get_id()` or `$TYM_ID`, and also listen to D-Bus signal/method call on the path `/me/endaaman/tym<ID>` and the interface name `me.endaaman.tym`.

### Signals

| Name | Input(D-Bus signature) | Description |
| ---- | --- | --- |
| `hook` | `s` | Triggers `signal` hook. |

For example, when you prepare the following config and command,

```lua
local tym = require('tym')
tym.set_hook('signal', function (p)
  print('Hello from DBus signal')
  print('param:', p)
end)
```

```
$ dbus-send /me/endaaman/tym0 me.endaaman.tym.hook string:'THIS IS PARAM'
```

you will get an output like below.

```
Hello from DBus signal
param:  THIS IS PARAM
```

Alternatively, you can use `tym` command to send signal.

```
$ tym --signal hook --dest 0 --param 'THIS IS PARAM'
```

If the target window is its own one, it will the value of `$TYM_ID` and `--dest` can be omitted. So it is enough like below.

```
$ tym --signal hook --param 'THIS IS PARAM'
```


### Methods

| Name | Input (D-Bus signature) | Output (D-Bus signature) | Description |
| ---- | --- | --- | --- |
| `get_ids` | None | `ai` | Get all tym instance IDs. |
| `echo` | `s` | `s` | Echo output the same as input. |
| `eval` | `s` | `s` | Evaluate one line lua script. `return` is needed. |
| `eval_file` | `s` | `s` | Evaluate a script file. `return` is needed. |
| `exec` | `s` | None | Execute one line lua script without outputs. |
| `eval_file` | `s` | None | Execute a script filt without outputs. |


For example, when you exec the command,

```
$ dbus-send --print-reply --type=method_call --dest=me.endaaman.tym /me/endaaman/tym0 me.endaaman.tym.eval string:'return "title is " .. tym.get("title")'
```

then you will get like below.

```
method return time=1646287109.007168 sender=:1.3633 -> destination=:1.3648 serial=39 reply_serial=2
   string "title is tym"
```

As same as signals, you can use `tym` command to execute method calling.

```
$ tym --call eval --dest 0 --param 'return "title is " .. tym.get("title")'
```

Of course, `--dest` can be omitted as well.


## Options

### `--help` `-h`

```
$ tym -h
```

### `--use=<path>` `-u <path>`

```
$ tym --use=/path/to/config.lua
```

If `NONE` is provided, all config will be default (user-defined config file will not be loaded).

```
$ tym -u NONE
```

### `--theme=<path>` `-t <path>`

```
$ tym --use=/path/to/theme.lua
```

If `NONE` is provided, default theme will be used.

```
$ tym -t NONE
```

### `--signal=<signal name>` `-s <signal name>`

```
$ tym --signal hook
```

Sends a D-Bus signal to the current instance (determined by `$TYM_ID` environment value). To send to another instance, use `--dest` (or `-d`) option.


### `--call=<method name>` `-c <method name>`

Calls D-Bus method of the current instance (determined by `$TYM_ID` environment value). To call it of another instance, provide `--dest` (or `-d`) option.

```
$ tym --call eval --param 'return 1 + 2'
```

### `--<config option>`

You can set config value via command line option.

```console
$ tym --shell=/bin/zsh --color_background=red --width=40 --ignore_default_keymap
```

## Development

Clone this repo and run as below

```console
$ autoreconf -fvi
$ ./configure --enable-debug
$ make && ./src/tym -u ./path/to/config.lua   # for debug
$ make check; cat src/tym-test.log            # for unit tests
```

Run tests in docker container

```console
$ docker build -t tym .
$ docker run tym
```

## Pro tips

<details><summary>Enable sixel support(for Arch Linux users)</summary>
<div>

### Build latest VTE with sixel support enabled

Install latest VTE.

```
$ yay -S vte3-git --editmenu
```

Edit `PKGBUILD` to add `-D sixel=true` option in `prepare()` function like the following.

```
prepare() {
  arch-meson vte build \
      -D sixel=true -D b_lto=false
}
```

And you should install `tym-git` package, since new features about sixel might be always on HEAD.

</div>
</details>

<details><summary>Scroll mouse wheel to set window transparency/font scale</summary>
<div>

You can increase/decrease window transparency by pressing `Ctrl + Shift + Down` / `Ctrl + Shift + Up` or `Shift` + mouse wheel and increase/decrease font scale by `Ctrl` + mouse wheel.

```lua
local tym = require('tym')

function update_alpha(delta)
  r, g, b, a = tym.color_to_rgba(tym.get('color_background'))
  a = math.max(math.min(1.0, a + delta), 0.0)
  bg = tym.rgba_to_color(r, g, b, a)
  tym.set('color_background', bg)
  tym.notify(string.format('%s alpha to %f', (delta > 0 and 'Inc' or 'Dec'), a))
end

tym.set_keymaps({
  ['<Ctrl><Shift>Up'] = function()
    update_alpha(0.05)
  end,
  ['<Ctrl><Shift>Down'] = function()
    update_alpha(-0.05)
  end,
})

tym.set_hook('scroll', function(dx, dy, x, y)
  if tym.check_mod_state('<Ctrl>') then
    if dy > 0 then
      s = tym.get('scale') - 10
    else
      s = tym.get('scale') + 10
    end
    tym.set('scale', s)
    tym.notify('set scale: ' .. s .. '%')
    return true
  end
  if tym.check_mod_state('<Shift>') then
    update_alpha(dy < 0 and 0.05 or -0.05)
    return true
  end
end)
```

</div>
</details>


<details><summary>Automated</summary>
<div>

```lua
local tym = require('tym')

tym.set_config({
  shell = '/bin/bash',
})

tym.set_timeout(coroutine.wrap(function()
  local i = 0
  local message = 'echo "hello World!"'
  while i < #message do
    i = i + 1
    tym.put(message:sub(i, i))
    coroutine.yield(true)
  end
  coroutine.yield(true)
  tym.send_key('<Ctrl>m')
  coroutine.yield(false)
end), 100)
```

</div>
</details>

<details><summary>Prevent tmux's key inputs delay</summary>
<div>

Key inputs alt-h,j,k,l for switching tmux's pane is often delayed when tmux is busy. This is the fix for it.

```lua
local tym = require('tym')

local remap = function (a, b)
  tym.set_keymap(a, function()
    tym.send_key(b)
  end)
end
remap('<Alt>h', '<Alt>Left') -- remap as meta key inputs
remap('<Alt>l', '<Alt>Right')
remap('<Alt><Shift>h', '<Alt><Shift>Left')
remap('<Alt><Shift>l', '<Alt><Shift>Right')
```

</div>
</details>

## License

MIT
