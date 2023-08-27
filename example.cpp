#include "seag.hpp"

int main(int argc, char* argv[])
{    
    //Create a new window.
    seag::Window window("example", 800, 500);

    //Change window font.
    window.setFont(new seag::Font("ubuntu_mono_atlas_alpha.png", 32));

    //Change window default background color.
    window.setBackgroundColor(sf::Color(0x191f2aff));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.getNativeWindow().setView(sf::View(visibleArea));
            }
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Up)
                    window.setFontSize(window.getFontSize() + 1);

                if(event.key.code == sf::Keyboard::Down)
                    window.setFontSize(std::max(1, window.getFontSize() - 1));
            }
        }

        window.clear();

        window.print("#bbold#r\n");
        window.print("#iitalic#r\n");
        window.print("#ff0000red#r\n");
        window.print("#0000ff#_00ff00blue on green#r\n");
        window.print(U"ζ\n");

        for(int i = 0; i < 50; i++)
        {
            window.pushBackgroundColor(sf::Color((i/50.f) * 255, 0, 0));
            window.print(U" ");
        }
        window.print("\n\n");
        window.resetStyle();

        window.print(U"#000000#_ffffff↑ zoom in#r\t");
        window.print(U"#000000#_ffffff↓ zoom out#r\n");

        window.display();
    }

    return 0;
}