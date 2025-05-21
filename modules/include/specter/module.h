#ifndef SPECTER_MODULE_H
#define SPECTER_MODULE_H

/* --------------------------------- Standard ------------------------------- */
#include <memory>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/mark/marker.h"
#include "specter/search/searcher.h"
#include "specter/server/server.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------- SpecterModule ----------------------------- */

class LIB_SPECTER_API SpecterModule {
 public:
  [[nodiscard]] static SpecterModule& getInstance();
  static void deleteInstance();

 public:
  ~SpecterModule();

  [[nodiscard]] Server& getServer() const;
  [[nodiscard]] Marker& getMarker() const;
  [[nodiscard]] Searcher& getSearcher() const;

 protected:
  explicit SpecterModule();

 private:
  static std::unique_ptr<SpecterModule> m_instance;

  std::unique_ptr<Server> m_server;
  std::unique_ptr<Marker> m_marker;
  std::unique_ptr<Searcher> m_searcher;
};

inline Server& server() { return SpecterModule::getInstance().getServer(); }

inline Marker& marker() { return SpecterModule::getInstance().getMarker(); }

inline Searcher& searcher() { return SpecterModule::getInstance().getSearcher(); }

}  // namespace specter

#endif  // SPECTER_MODULE_H