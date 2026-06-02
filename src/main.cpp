#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

struct Key {
    const char* note;
    sf::Keyboard::Key button;
    bool pressed = false;
};

struct Button {
    sf::FloatRect area;
    const char* label;
};

bool isBlack(const char* note)
{
    return std::string(note).find('b') != std::string::npos;
}

int whiteKeyBefore(const std::vector<Key>& keys, std::size_t index)
{
    int count = 0;
    for (std::size_t i = 0; i < index; ++i) {
        if (!isBlack(keys[i].note)) {
            ++count;
        }
    }
    return count;
}

void setPianoVolume(const std::vector<sf::SoundBuffer>& originalBuffers,
                    std::vector<sf::SoundBuffer>& buffers,
                    std::vector<sf::Sound>& sounds,
                    float volume)
{
    const float gain = std::max(1.f, volume / 100.f);
    const float sfmlVolume = std::min(volume, 100.f);

    for (std::size_t i = 0; i < buffers.size(); ++i) {
        const std::int16_t* samples = originalBuffers[i].getSamples();
        const std::uint64_t sampleCount = originalBuffers[i].getSampleCount();

        if (sampleCount == 0) {
            continue;
        }

        std::vector<std::int16_t> louderSamples(sampleCount);
        for (std::uint64_t sample = 0; sample < sampleCount; ++sample) {
            const int louder = static_cast<int>(static_cast<float>(samples[sample]) * gain);
            louderSamples[sample] = static_cast<std::int16_t>(std::clamp(louder, -32768, 32767));
        }

        const bool loaded = buffers[i].loadFromSamples(louderSamples.data(),
                                                       sampleCount,
                                                       originalBuffers[i].getChannelCount(),
                                                       originalBuffers[i].getSampleRate(),
                                                       originalBuffers[i].getChannelMap());
        if (!loaded) {
            std::cout << "Could not amplify sound buffer\n";
        }
        sounds[i].setVolume(sfmlVolume);
    }
}

int main()
{
    std::vector<Key> keys = {
        {"C3", sf::Keyboard::Key::Num1},  {"Db3", sf::Keyboard::Key::Num2},
        {"D3", sf::Keyboard::Key::Num3},  {"Eb3", sf::Keyboard::Key::Num4},
        {"E3", sf::Keyboard::Key::Num5},  {"F3", sf::Keyboard::Key::Num6},
        {"Gb3", sf::Keyboard::Key::Num7}, {"G3", sf::Keyboard::Key::Num8},
        {"Ab3", sf::Keyboard::Key::Num9}, {"A3", sf::Keyboard::Key::Num0},
        {"Bb3", sf::Keyboard::Key::Q},    {"B3", sf::Keyboard::Key::W},
        {"C4", sf::Keyboard::Key::E},     {"Db4", sf::Keyboard::Key::R},
        {"D4", sf::Keyboard::Key::T},     {"Eb4", sf::Keyboard::Key::Y},
        {"E4", sf::Keyboard::Key::U},     {"F4", sf::Keyboard::Key::I},
        {"Gb4", sf::Keyboard::Key::O},    {"G4", sf::Keyboard::Key::P},
        {"Ab4", sf::Keyboard::Key::A},    {"A4", sf::Keyboard::Key::S},
        {"Bb4", sf::Keyboard::Key::D},    {"B4", sf::Keyboard::Key::F},
        {"C5", sf::Keyboard::Key::G},     {"Db5", sf::Keyboard::Key::H},
        {"D5", sf::Keyboard::Key::J},     {"Eb5", sf::Keyboard::Key::K},
        {"E5", sf::Keyboard::Key::L},     {"F5", sf::Keyboard::Key::Semicolon},
        {"Gb5", sf::Keyboard::Key::Z},    {"G5", sf::Keyboard::Key::X},
        {"Ab5", sf::Keyboard::Key::C},    {"A5", sf::Keyboard::Key::V},
        {"Bb5", sf::Keyboard::Key::B},    {"B5", sf::Keyboard::Key::N},
        {"C6", sf::Keyboard::Key::M},     {"Db6", sf::Keyboard::Key::Comma},
        {"D6", sf::Keyboard::Key::Period}, {"Eb6", sf::Keyboard::Key::Slash}
    };

    std::vector<sf::SoundBuffer> originalBuffers(keys.size());
    std::vector<sf::SoundBuffer> buffers(keys.size());
    std::vector<sf::Sound> sounds;
    sounds.reserve(keys.size());
    float volume = 100.f;

    for (std::size_t i = 0; i < keys.size(); ++i) {
        if (!originalBuffers[i].loadFromFile(std::string("Piano sounds/") + keys[i].note + ".mp3")) {
            std::cout << "Missing sound: " << keys[i].note << '\n';
        }
        buffers[i] = originalBuffers[i];
        sounds.emplace_back(buffers[i]);
    }

    setPianoVolume(originalBuffers, buffers, sounds, volume);

    const float windowWidth = 1180.f;
    const float windowHeight = 420.f;
    const float top = 78.f;
    const float whiteHeight = 310.f;
    const float blackHeight = 185.f;
    const float whiteWidth = windowWidth / 23.f;
    const float blackWidth = whiteWidth * 0.62f;
    const Button volumeDown {{{24.f, 18.f}, {52.f, 34.f}}, "-"};
    const Button volumeUp {{{86.f, 18.f}, {52.f, 34.f}}, "+"};
    const Button quitButton {{{1044.f, 18.f}, {132.f, 34.f}}, "Quit"};

    sf::Font font("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    sf::RenderWindow window(sf::VideoMode({1180, 420}), "Piano Keys");
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                for (std::size_t i = 0; i < keys.size(); ++i) {
                    if (keyPressed->code == keys[i].button) {
                        keys[i].pressed = true;
                        sounds[i].stop();
                        sounds[i].play();
                    }
                }
            }

            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                for (std::size_t i = 0; i < keys.size(); ++i) {
                    if (keyReleased->code == keys[i].button) {
                        keys[i].pressed = false;
                    }
                }
            }

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                const sf::Vector2f mouse {
                    static_cast<float>(mousePressed->position.x),
                    static_cast<float>(mousePressed->position.y)
                };

                if (volumeDown.area.contains(mouse) && volume > 0.f) {
                    volume -= 10.f;
                }

                if (volumeUp.area.contains(mouse) && volume < 300.f) {
                    volume += 10.f;
                }

                if (quitButton.area.contains(mouse)) {
                    window.close();
                }

                setPianoVolume(originalBuffers, buffers, sounds, volume);
            }
        }

        window.clear(sf::Color(24, 28, 34));

        sf::RectangleShape base({windowWidth, windowHeight});
        base.setFillColor(sf::Color(18, 20, 24));
        window.draw(base);

        auto drawButton = [&](const Button& button) {
            sf::RectangleShape rect({button.area.size.x, button.area.size.y});
            rect.setPosition(button.area.position);
            rect.setFillColor(sf::Color(48, 56, 66));
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor(sf::Color(95, 105, 118));
            window.draw(rect);

            sf::Text text(font, button.label, 18);
            text.setPosition({button.area.position.x + 16.f, button.area.position.y + 6.f});
            text.setFillColor(sf::Color(235, 238, 242));
            window.draw(text);
        };

        drawButton(volumeDown);
        drawButton(volumeUp);
        drawButton(quitButton);

        sf::Text volumeText(font, "Volume: " + std::to_string(static_cast<int>(volume)), 18);
        volumeText.setPosition({154.f, 24.f});
        volumeText.setFillColor(sf::Color(225, 230, 236));
        window.draw(volumeText);

        for (std::size_t i = 0; i < keys.size(); ++i) {
            if (isBlack(keys[i].note)) {
                continue;
            }

            const float x = static_cast<float>(whiteKeyBefore(keys, i)) * whiteWidth;
            sf::RectangleShape rect({whiteWidth - 2.f, whiteHeight});
            rect.setPosition({x + 1.f, top});
            rect.setFillColor(sf::Color(238, 236, 226));
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor(sf::Color(120, 122, 126));
            if (keys[i].pressed) {
                rect.setFillColor(sf::Color(118, 190, 218));
            }
            window.draw(rect);
        }

        for (std::size_t i = 0; i < keys.size(); ++i) {
            if (!isBlack(keys[i].note)) {
                continue;
            }

            const float x = static_cast<float>(whiteKeyBefore(keys, i)) * whiteWidth - blackWidth / 2.f;
            sf::RectangleShape rect({blackWidth, blackHeight});
            rect.setPosition({x, top});
            rect.setFillColor(keys[i].pressed ? sf::Color(55, 135, 178) : sf::Color(22, 23, 27));
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor(sf::Color(8, 9, 12));
            window.draw(rect);
        }

        window.display();
    }
}
