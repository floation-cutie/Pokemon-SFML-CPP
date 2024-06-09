#include "../include/ResourceHolder.hpp"
#include "../include/ResourceIdentifiers.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

// be careful about template
template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load(Identifier id,
                                                const std::string &filename) {
  auto resource(std::make_unique<Resource>());
  if (!resource->loadFromFile(filename))
    throw std::runtime_error("ResourceHolder::load - Failed to \
 load " + filename);
  insertResource(id, std::move(resource));
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::insertResource(
    Identifier id, std::unique_ptr<Resource> resource) {
  auto inserted = mResourceMap.insert(std::make_pair(id, std::move(resource)));
  assert(inserted.second);
}

template <typename Resource, typename Identifier>
template <typename Parameter>
void ResourceHolder<Resource, Identifier>::load(Identifier id,
                                                const std::string &filename,
                                                const Parameter &secondParam) {
  // Create and load resource
  auto resource(std::make_unique<Resource>());
  if (!resource->loadFromFile(filename, secondParam))
    throw std::runtime_error("ResourceHolder::load - Failed to load " +
                             filename);

  // If loading successful, insert resource to map
  insertResource(id, std::move(resource));
}

template <typename Resource, typename Identifier>
Resource &ResourceHolder<Resource, Identifier>::get(Identifier id) {
  auto found = mResourceMap.find(id);
  assert(found != mResourceMap.end());

  return *found->second;
}

template <typename Resource, typename Identifier>
const Resource &ResourceHolder<Resource, Identifier>::get(Identifier id) const {
  auto found = mResourceMap.find(id);
  assert(found != mResourceMap.end());

  return *found->second;
}

template class ResourceHolder<sf::Texture, Textures::ID>;
template class ResourceHolder<sf::Font, Fonts::ID>;
