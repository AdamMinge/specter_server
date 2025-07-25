/* ----------------------------------- Local -------------------------------- */
#include "specter/module.h"

#include "specter/search/strategy.h"
#include "specter/service/marker.h"
#include "specter/service/object.h"
#include "specter/service/recorder.h"
/* -------------------------------------------------------------------------- */

namespace specter {

std::unique_ptr<SpecterModule> SpecterModule::m_instance =
  std::unique_ptr<SpecterModule>(nullptr);

SpecterModule &SpecterModule::getInstance() {
  if (!m_instance) m_instance.reset(new SpecterModule);

  return *m_instance;
}

void SpecterModule::deleteInstance() { m_instance.reset(nullptr); }

SpecterModule::SpecterModule()
    : m_server(std::make_unique<Server>()),
      m_marker(std::make_unique<Marker>()),
      m_searcher(std::make_unique<Searcher>()) {
  m_server->registerService<RecorderService>();
  m_server->registerService<MarkerService>();
  m_server->registerService<ObjectService>();

  m_searcher->addStrategy(std::make_unique<TypeSearch>());
  m_searcher->addStrategy(std::make_unique<PropertiesSearch>());
  m_searcher->addStrategy(std::make_unique<PathSearch>());
  m_searcher->addStrategy(std::make_unique<OrderIndexSearch>());
  m_searcher->addStrategy(std::make_unique<SessionIdSearch>());
}

SpecterModule::~SpecterModule() = default;

Server &SpecterModule::getServer() const { return *m_server; }

Marker &SpecterModule::getMarker() const { return *m_marker; }

Searcher &SpecterModule::getSearcher() const { return *m_searcher; }

}// namespace specter
