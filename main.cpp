#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <assert.h>


char piano_sound_path[] = "../Piano sounds/___.mp3";
// 16, 17, 18 for note

void blend_note(char* const full_path, const char* note)
{
    assert(full_path != nullptr && note != nullptr);

    full_path[16] = note[0];
    full_path[17] = note[1];
    full_path[18] = note[2];
}

struct Key 
{
    const char* note;
    sf::Keyboard::Key button;
    bool isBlack;
    bool pressed = false;
};

struct Button 
{
    sf::FloatRect area;
    const char* label;
};


// very expensive
bool isBlack(const char* note)
{
    return std::string(note).find('b') != std::string::npos;
}
// -------------- replace with a boolean in key itself


/// expensive-repititive as being used
/// @deprecated
int whiteKeysBefore(const std::vector<Key>& keys, std::size_t index)
{
    int count = 0;
    for (std::size_t i = 0; i < index; ++i) {
        if (!keys[i].isBlack) { // black-change changed here
            ++count;
        }
    }
    return count;
}
// incorporated inline effectively and optimally

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
        {"C3_", sf::Keyboard::Key::Num1, false},  {"Db3", sf::Keyboard::Key::Num2, true},
        {"D3_", sf::Keyboard::Key::Num3, false},  {"Eb3", sf::Keyboard::Key::Num4, true},
        {"E3_", sf::Keyboard::Key::Num5, false},  {"F3_", sf::Keyboard::Key::Num6, false},
        {"Gb3", sf::Keyboard::Key::Num7, true},   {"G3_", sf::Keyboard::Key::Num8, false},
        {"Ab3", sf::Keyboard::Key::Num9, true},   {"A3_", sf::Keyboard::Key::Num0, false},
        {"Bb3", sf::Keyboard::Key::Q, true},      {"B3_", sf::Keyboard::Key::W, false},
        {"C4_", sf::Keyboard::Key::E, false},     {"Db4", sf::Keyboard::Key::R, true},
        {"D4_", sf::Keyboard::Key::T, false},     {"Eb4", sf::Keyboard::Key::Y, true},
        {"E4_", sf::Keyboard::Key::U, false},     {"F4_", sf::Keyboard::Key::I, false},
        {"Gb4", sf::Keyboard::Key::O, true},      {"G4_", sf::Keyboard::Key::P, false},
        {"Ab4", sf::Keyboard::Key::A, true},      {"A4_", sf::Keyboard::Key::S, false},
        {"Bb4", sf::Keyboard::Key::D, true},      {"B4_", sf::Keyboard::Key::F, false},
        {"C5_", sf::Keyboard::Key::G, false},     {"Db5", sf::Keyboard::Key::H, true},
        {"D5_", sf::Keyboard::Key::J, false},     {"Eb5", sf::Keyboard::Key::K, true},
        {"E5_", sf::Keyboard::Key::L, false},     {"F5_", sf::Keyboard::Key::Semicolon, false},
        {"Gb5", sf::Keyboard::Key::Z, true},      {"G5_", sf::Keyboard::Key::X, false},
        {"Ab5", sf::Keyboard::Key::C, true},      {"A5_", sf::Keyboard::Key::V, false},
        {"Bb5", sf::Keyboard::Key::B, true},      {"B5_", sf::Keyboard::Key::N, false},
        {"C6_", sf::Keyboard::Key::M, false},     {"Db6", sf::Keyboard::Key::Comma, true},
        {"D6_", sf::Keyboard::Key::Period, false},{"Eb6", sf::Keyboard::Key::Slash, true}
    };

    std::vector<sf::SoundBuffer> originalBuffers(keys.size());
    std::vector<sf::SoundBuffer> buffers(keys.size());
    std::vector<sf::Sound> sounds;
    sounds.reserve(keys.size());

    float volume = 100.f;

    for (std::size_t i = 0; i < keys.size(); ++i) 
    {
        // change into O(1) by adding underscore into the file name
        blend_note(piano_sound_path, keys[i].note);
        if (!originalBuffers[i].loadFromFile(piano_sound_path)) { 
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

    sf::Font font("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

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

                if (volumeDown.area.contains(mouse)) {
                    (volume > 0.f) && (volume -= 10.f);
                }

                else if (volumeUp.area.contains(mouse)) {
                    (volume < 300.f) && (volume += 10.f);
                }

                else if (quitButton.area.contains(mouse)) {
                    window.close();
                }

                std::clamp(volume, 0.f, 300.f);
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

        for (std::size_t i = 0, white_keys = 0; i < keys.size(); ++i) 
        {
            if (keys[i].isBlack) { // black-change changed here
                const float x = static_cast<float>(whiteKeysBefore(keys, i)) * whiteWidth - blackWidth / 2.f;
                sf::RectangleShape rect({blackWidth, blackHeight});
                rect.setPosition({x, top});
                rect.setFillColor(keys[i].pressed ? sf::Color(55, 135, 178) : sf::Color(22, 23, 27));
                rect.setOutlineThickness(1.f);
                rect.setOutlineColor(sf::Color(8, 9, 12));
                window.draw(rect);
            }

            else 
            {
                // const float x = static_cast<float>(whiteKeysBefore(keys, i)) * whiteWidth; // expensive and repititive
                const float x = static_cast<float>(white_keys) * whiteWidth;                  // 
                sf::RectangleShape rect({whiteWidth - 2.f, whiteHeight});                     //             
                rect.setPosition({x + 1.f, top});                                             //
                rect.setFillColor(sf::Color(238, 236, 226));                                  //
                rect.setOutlineThickness(1.f);                                                //
                rect.setOutlineColor(sf::Color(120, 122, 126));                               //     
                if (keys[i].pressed) {                                                        //
                    rect.setFillColor(sf::Color(118, 190, 218));                              //
                }                                                                             //
                window.draw(rect);                                                            //
                white_keys += 1;                                                              // incorporated
            }
        }

        // incorporated above in the one and only iteration
        //for (std::size_t i = 0; i < keys.size(); ++i) {
        //    if (!keys[i].isBlack) { // black-change 
        //        continue;
        //    }

        //    const float x = static_cast<float>(whiteKeysBefore(keys, i)) * whiteWidth - blackWidth / 2.f;
        //    sf::RectangleShape rect({blackWidth, blackHeight});
        //    rect.setPosition({x, top});
        //    rect.setFillColor(keys[i].pressed ? sf::Color(55, 135, 178) : sf::Color(22, 23, 27));
        //    rect.setOutlineThickness(1.f);
        //    rect.setOutlineColor(sf::Color(8, 9, 12));
        //    window.draw(rect);
        //}
        // ----------------------------

        window.display();
    }
}
