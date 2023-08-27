# seag
A simple engine for ascii game (using SFML).

## Installation

Download:
* [seag.hpp](https://github.com/Xorrad/seag/seag.hpp)
* [seag.cpp](https://github.com/Xorrad/seag/seag.cpp)
* [seag_atlas.cpp](https://github.com/Xorrad/seag/seag_atlas.cpp)

Then Install SFML and link it to your project.

## Documentation

Include the file:
```cpp
#include "seag.hpp"
```

Create a window and print stuff:
```cpp
seag::Window window("example", 800, 500);

window.clear();
window.print("hello world\n");
window.display();
```

Change foreground and background colors:
```cpp
//Change active brush colors.
window.pushForegroundColor(sf::Color(r, g, b));
window.pushBackgroundColor(sf::Color(r, g, b));

//Reset to default brush.
window.resetStyle();
```

Use colors in text:
```cpp
//Use #FF0000 to change foreground color.
//Use #_FF0000 to change background color.
//Use #r to reset to default colors.
window.print("#FF0000 print in red #r\n");
```

Use text styles:
```cpp
//Use #b or #i to toggle text styles. To disable, #r will also work.
window.print("#b bold #b\n");
window.print("#i italic #i\n");
```

Keep the window open:
```cpp
while (window.isOpen())
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}
```

## Changelog

### 2023/08/27 - v0.1

Additions:
* [+] Window creation.
* [+] Bitmap atlas.
* [+] Text rendering.
* [+] Foreground and background colors.
* [+] Bold and italic text styles.