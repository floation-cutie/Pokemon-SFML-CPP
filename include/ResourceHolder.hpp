#ifndef RESOURCEHOLDER_HPP_
#define RESOURCEHOLDER_HPP_
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cassert>
#include <map>
#include <memory>

template <typename Resource, typename Identifier> class ResourceHolder {
private:
  std::map<Identifier, std::unique_ptr<Resource>> mResourceMap;

public:
  void load(Identifier id, const std::string &filename);
  template <typename Parameter>
  void load(Identifier id, const std::string &filename,
            const Parameter &secondParam);
  Resource &get(Identifier id);
  const Resource &get(Identifier id) const;

private:
  void insertResource(Identifier id, std::unique_ptr<Resource> resource);
};
//#include "./ResourceHolder.inl"
#endif // !RESOURCEHOLDER_HPP_
