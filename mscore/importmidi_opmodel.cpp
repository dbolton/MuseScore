#include "importmidi_opmodel.h"
#include "importmidi_operations.h"


namespace Ms {

struct Node {
      QString name;
      MidiOperation oper;
      QStringList values;
      bool visible = true;
      Node *parent = nullptr;
      std::vector<std::unique_ptr<Node> > children;
      };

struct Controller {
      Node *LHRHdoIt = nullptr;
      Node *LHRHMethod = nullptr;
      Node *LHRHPitchOctave = nullptr;
      Node *LHRHPitchNote = nullptr;
      Node *quantValue = nullptr;
      Node *quantHuman = nullptr;
      Node *searchTuplets = nullptr;
      Node *duplets = nullptr;
      Node *triplets = nullptr;
      Node *quadruplets = nullptr;
      Node *quintuplets = nullptr;
      Node *septuplets = nullptr;
      Node *nonuplets = nullptr;

      int trackCount = 0;
      bool updateNodeDependencies(Node *node, bool force_update);
      };

OperationsModel::OperationsModel()
            : root(std::unique_ptr<Node>(new Node()))
            , controller(std::unique_ptr<Controller>(new Controller()))
      {
      beginResetModel();
                  // - initialize opeations with their default values
                  // - string lists below should match Operation enum values
      Node *quantValue = new Node;
      quantValue->name = "Quantization";
      quantValue->oper.type = MidiOperation::Type::QUANT_VALUE;
      quantValue->oper.value = (int)TrackOperations().quantize.value;
      quantValue->values.push_back("Value from preferences");
      quantValue->values.push_back("Quarter");
      quantValue->values.push_back("Eighth");
      quantValue->values.push_back("16th");
      quantValue->values.push_back("32nd");
      quantValue->values.push_back("64th");
      quantValue->values.push_back("128th");
      quantValue->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(quantValue));
      controller->quantValue = quantValue;


      Node *reduceToShorter = new Node;
      reduceToShorter->name = "Reduce to shortest note in bar";
      reduceToShorter->oper.type = MidiOperation::Type::QUANT_REDUCE;
      reduceToShorter->oper.value = Quantization().reduceToShorterNotesInBar;
      reduceToShorter->parent = quantValue;
      quantValue->children.push_back(std::unique_ptr<Node>(reduceToShorter));


//      Node *humanPerformance = new Node;
//      humanPerformance->name = "Human performance";
//      humanPerformance->oper.type = MidiOperation::Type::QUANT_HUMAN;
//      humanPerformance->oper.value = Quantization().humanPerformance;
//      humanPerformance->parent = quantValue;
//      quantValue->children.push_back(std::unique_ptr<Node>(humanPerformance));
//      controller->quantHuman = humanPerformance;


      Node *useDots = new Node;
      useDots->name = "Use dots";
      useDots->oper.type = MidiOperation::Type::USE_DOTS;
      useDots->oper.value = TrackOperations().useDots;
      useDots->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(useDots));

      Node *useMultipleVoices = new Node;
      useMultipleVoices->name = "Multiple voices";
      useMultipleVoices->oper.type = MidiOperation::Type::USE_MULTIPLE_VOICES;
      useMultipleVoices->oper.value = TrackOperations().useMultipleVoices;
      useMultipleVoices->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(useMultipleVoices));


      // ------------- tuplets --------------

      Node *searchTuplets = new Node;
      searchTuplets->name = "Search tuplets";
      searchTuplets->oper.type = MidiOperation::Type::TUPLET_SEARCH;
      searchTuplets->oper.value = TrackOperations().tuplets.doSearch;
      searchTuplets->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(searchTuplets));
      controller->searchTuplets = searchTuplets;


      Node *duplets = new Node;
      duplets->name = "Duplets (2)";
      duplets->oper.type = MidiOperation::Type::TUPLET_2;
      duplets->oper.value = TrackOperations().tuplets.duplets;
      duplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(duplets));
      controller->duplets = duplets;


      Node *triplets = new Node;
      triplets->name = "Triplets (3)";
      triplets->oper.type = MidiOperation::Type::TUPLET_3;
      triplets->oper.value = TrackOperations().tuplets.triplets;
      triplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(triplets));
      controller->triplets = triplets;


      Node *quadruplets = new Node;
      quadruplets->name = "Quadruplets (4)";
      quadruplets->oper.type = MidiOperation::Type::TUPLET_4;
      quadruplets->oper.value = TrackOperations().tuplets.quadruplets;
      quadruplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(quadruplets));
      controller->quadruplets = quadruplets;


      Node *quintuplets = new Node;
      quintuplets->name = "Quintuplets (5)";
      quintuplets->oper.type = MidiOperation::Type::TUPLET_5;
      quintuplets->oper.value = TrackOperations().tuplets.quintuplets;
      quintuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(quintuplets));
      controller->quintuplets = quintuplets;


      Node *septuplets = new Node;
      septuplets->name = "Septuplets (7)";
      septuplets->oper.type = MidiOperation::Type::TUPLET_7;
      septuplets->oper.value = TrackOperations().tuplets.septuplets;
      septuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(septuplets));
      controller->septuplets = septuplets;


      Node *nonuplets = new Node;
      nonuplets->name = "Nonuplets (9)";
      nonuplets->oper.type = MidiOperation::Type::TUPLET_9;
      nonuplets->oper.value = TrackOperations().tuplets.nonuplets;
      nonuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(nonuplets));
      controller->nonuplets = nonuplets;

      // ------------------------------------


      Node *doLHRH = new Node;
      doLHRH->name = "LH/RH separation";
      doLHRH->oper.type = MidiOperation::Type::DO_LHRH_SEPARATION;
      doLHRH->oper.value = LHRHSeparation().doIt;
      doLHRH->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(doLHRH));
      controller->LHRHdoIt = doLHRH;


      Node *LHRHMethod = new Node;
      LHRHMethod->name = "Separation method";
      LHRHMethod->oper.type = MidiOperation::Type::LHRH_METHOD;
      LHRHMethod->oper.value = (int)LHRHSeparation().method;
      LHRHMethod->values.push_back("Hand width");
      LHRHMethod->values.push_back("Fixed pitch");
      LHRHMethod->parent = doLHRH;
      doLHRH->children.push_back(std::unique_ptr<Node>(LHRHMethod));
      controller->LHRHMethod = LHRHMethod;


      Node *LHRHPitchOctave = new Node;
      LHRHPitchOctave->name = "Split pitch octave";
      LHRHPitchOctave->oper.type = MidiOperation::Type::LHRH_SPLIT_OCTAVE;
      LHRHPitchOctave->oper.value = (int)LHRHSeparation().splitPitchOctave;
      LHRHPitchOctave->values.push_back("C-1");
      LHRHPitchOctave->values.push_back("C0");
      LHRHPitchOctave->values.push_back("C1");
      LHRHPitchOctave->values.push_back("C2");
      LHRHPitchOctave->values.push_back("C3");
      LHRHPitchOctave->values.push_back("C4");
      LHRHPitchOctave->values.push_back("C5");
      LHRHPitchOctave->values.push_back("C6");
      LHRHPitchOctave->values.push_back("C7");
      LHRHPitchOctave->values.push_back("C8");
      LHRHPitchOctave->values.push_back("C9");
      LHRHPitchOctave->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchOctave));
      controller->LHRHPitchOctave = LHRHPitchOctave;


      Node *LHRHPitchNote = new Node;
      LHRHPitchNote->name = "Split pitch note";
      LHRHPitchNote->oper.type = MidiOperation::Type::LHRH_SPLIT_NOTE;
      LHRHPitchNote->oper.value = (int)LHRHSeparation().splitPitchNote;
      LHRHPitchNote->values.push_back("C");
      LHRHPitchNote->values.push_back("C#");
      LHRHPitchNote->values.push_back("D");
      LHRHPitchNote->values.push_back("D#");
      LHRHPitchNote->values.push_back("E");
      LHRHPitchNote->values.push_back("F");
      LHRHPitchNote->values.push_back("F#");
      LHRHPitchNote->values.push_back("G");
      LHRHPitchNote->values.push_back("G#");
      LHRHPitchNote->values.push_back("A");
      LHRHPitchNote->values.push_back("A#");
      LHRHPitchNote->values.push_back("B");
      LHRHPitchNote->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchNote));
      controller->LHRHPitchNote = LHRHPitchNote;

      //--------------------------------------------------------------------
      connect(this,
              SIGNAL(dataChanged(QModelIndex,QModelIndex)),
              SLOT(onDataChanged(QModelIndex)));
      controller->updateNodeDependencies(nullptr, true);
      endResetModel();
      }

OperationsModel::~OperationsModel()
      {
      }

void OperationsModel::reset(int trackCount)
      {
      controller->trackCount = trackCount;
      }

QModelIndex OperationsModel::index(int row, int column, const QModelIndex &parent) const
      {
      if (!root || row < 0 || column < 0 || column >= OperationCol::COL_COUNT)
            return QModelIndex();
      Node *parent_node = nodeFromIndex(parent);
      if (!parent_node)
            return QModelIndex();
      if (parent_node->children.empty() || row >= (int)parent_node->children.size())
            return QModelIndex();
                  // find new row in connection with invisible items
      int shift = 0;
      for (int i = 0; i <= row + shift; ++i) {
            if (i >= (int)parent_node->children.size())
                  return QModelIndex();
            if (!parent_node->children.at(i)->visible)
                  ++shift;
            }
      Node *child_node = parent_node->children.at(row + shift).get();
      if (!child_node || !child_node->visible)
            return QModelIndex();
      return createIndex(row, column, child_node);
      }

QModelIndex OperationsModel::parent(const QModelIndex &child) const
      {
      Node *node = nodeFromIndex(child);
      if (!node)
          return QModelIndex();
      Node *parent_node = node->parent;
      if (!parent_node)
          return QModelIndex();
      Node *grandparent_node = parent_node->parent;
      if (!grandparent_node)
          return QModelIndex();
      auto &v = grandparent_node->children;
      auto iter = std::find_if(v.begin(), v.end(),
          [parent_node](std::unique_ptr<Node> &el){ return el.get() == parent_node; });
      int row = (iter == v.end()) ? -1 : iter - v.begin();
      return createIndex(row, 0, parent_node);
      }

int OperationsModel::rowCount(const QModelIndex &parent) const
      {
      if (parent.column() >= OperationCol::COL_COUNT)
            return 0;
      Node *parent_node = nodeFromIndex(parent);
      if (!parent_node)
            return 0;
                  // take only visible nodes into account
      size_t counter = 0;
      for (const auto &p: parent_node->children)
            if (p->visible)
                  ++counter;
      return counter;
      }

int OperationsModel::columnCount(const QModelIndex &parent) const
      {
      return OperationCol::COL_COUNT;
      }

// All nodes can have either bool value or list of possible values
// also node value can be undefined (QVariant()), for example grayed checkbox

QVariant OperationsModel::data(const QModelIndex &index, int role) const
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return QVariant();
      switch (role) {
            case DataRole:
                  if (node->values.empty())  // checkbox
                        return node->oper.value.toBool();
                  else
                        return node->oper.value.toInt();
                  break;
            case Qt::DisplayRole:
                  switch (index.column()) {
                        case OperationCol::OPER_NAME:
                              return node->name;
                        case OperationCol::VALUE:
                              if (!node->values.empty()) {
                                    if (!node->oper.value.isValid()) // undefined operation value
                                          return " . . . ";
                                                // list contains names of possible string values
                                                // like {"1/4", "1/8"}
                                                // valid node value is one of enum items
                                                // -> use enum item as index
                                    int indexOfValue = node->oper.value.toInt();
                                    if (indexOfValue < node->values.size() && indexOfValue >= 0)
                                          return node->values.at(indexOfValue);
                                    }
                                          // otherwise return nothing because it's a checkbox
                              break;
                        default:
                              break;
                        }
                  break;
            case Qt::EditRole:
                  if (index.column() == OperationCol::VALUE && !node->values.empty())
                        return node->values;
                  break;
            case Qt::CheckStateRole:
                  if (index.column() == OperationCol::VALUE && node->values.empty()) {
                        if (!node->oper.value.isValid())
                              return Qt::PartiallyChecked;
                        return (node->oper.value.toBool())
                               ? Qt::Checked : Qt::Unchecked;
                        }
                  break;
            case Qt::SizeHintRole:
                  {
                  QSize sz;
                  sz.setHeight(22);
                  return sz;
                  }
            case OperationTypeRole:
                  return (int)node->oper.type;
            default:
                  break;
            }
      return QVariant();
      }

QVariant OperationsModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            {
            switch (section) {
                  case OperationCol::OPER_NAME:
                        return "Selected track [" + trackLabel + "] operations";
                  case OperationCol::VALUE:
                        return "Value";
                  default:
                        break;
                  }
            }
      return QVariant();
      }

Qt::ItemFlags OperationsModel::flags(const QModelIndex &index) const
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return Qt::ItemFlags();
      Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled);
      if (index.column() == OperationCol::VALUE) {
            if (node->values.empty())           // node value is bool - a checkbox
                  flags |= Qt::ItemIsUserCheckable;
            else        // node has list of values
                  flags |= Qt::ItemIsEditable;
            }
      return flags;
      }

bool OperationsModel::setData(const QModelIndex &index, const QVariant &value, int role)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return false;
      bool result = false;
      if (index.column() == OperationCol::VALUE) {
            switch (role) {
                  case Qt::CheckStateRole:
                        node->oper.value = value.toBool();
                        result = true;
                        break;
                  case Qt::EditRole:
                                    // set enum value from value == list index
                        node->oper.value = value.toInt();
                        result = true;
                        break;
                  default:
                        break;
                  }
            }
      if (result)
            emit dataChanged(index, index);
      return result;
      }

void setNodeOperations(Node *node, const DefinedTrackOperations &opers)
      {
      if (opers.undefinedOpers.contains((int)node->oper.type))
            node->oper.value = QVariant();
      else {
            switch (node->oper.type) {
                  case MidiOperation::Type::DO_IMPORT: break;

                  case MidiOperation::Type::QUANT_VALUE:
                        node->oper.value = (int)opers.opers.quantize.value; break;
                  case MidiOperation::Type::QUANT_REDUCE:
                        node->oper.value = opers.opers.quantize.reduceToShorterNotesInBar; break;
                  case MidiOperation::Type::QUANT_HUMAN:
                        node->oper.value = opers.opers.quantize.humanPerformance; break;

                  case MidiOperation::Type::DO_LHRH_SEPARATION:
                        node->oper.value = opers.opers.LHRH.doIt; break;
                  case MidiOperation::Type::LHRH_METHOD:
                        node->oper.value = (int)opers.opers.LHRH.method; break;
                  case MidiOperation::Type::LHRH_SPLIT_OCTAVE:
                        node->oper.value = (int)opers.opers.LHRH.splitPitchOctave; break;
                  case MidiOperation::Type::LHRH_SPLIT_NOTE:
                        node->oper.value = (int)opers.opers.LHRH.splitPitchNote; break;

                  case MidiOperation::Type::USE_DOTS:
                        node->oper.value = opers.opers.useDots; break;

                  case MidiOperation::Type::USE_MULTIPLE_VOICES:
                        node->oper.value = opers.opers.useMultipleVoices; break;

                  case MidiOperation::Type::TUPLET_SEARCH:
                        node->oper.value = opers.opers.tuplets.doSearch; break;
                  case MidiOperation::Type::TUPLET_2:
                        node->oper.value = opers.opers.tuplets.duplets; break;
                  case MidiOperation::Type::TUPLET_3:
                        node->oper.value = opers.opers.tuplets.triplets; break;
                  case MidiOperation::Type::TUPLET_4:
                        node->oper.value = opers.opers.tuplets.quadruplets; break;
                  case MidiOperation::Type::TUPLET_5:
                        node->oper.value = opers.opers.tuplets.quintuplets; break;
                  case MidiOperation::Type::TUPLET_7:
                        node->oper.value = opers.opers.tuplets.septuplets; break;
                  case MidiOperation::Type::TUPLET_9:
                        node->oper.value = opers.opers.tuplets.nonuplets; break;
                  }
            }
      for (const auto &nodePtr: node->children)
            setNodeOperations(nodePtr.get(), opers);
      }

void OperationsModel::setTrackData(const QString &trackLabel, const DefinedTrackOperations &opers)
      {
      this->trackLabel = trackLabel;
                  // set new operations values
      beginResetModel();
      for (const auto &nodePtr: root->children)
            setNodeOperations(nodePtr.get(), opers);
      controller->updateNodeDependencies(nullptr, true);
      endResetModel();
      }

void OperationsModel::onDataChanged(const QModelIndex &index)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return;
      if (controller->updateNodeDependencies(node, false))
            layoutChanged();
      }

Node* OperationsModel::nodeFromIndex(const QModelIndex &index) const
      {
      if (index.isValid())
            return static_cast<Node *>(index.internalPointer());
      else
            return root.get();
      }

// Different controller actions, i.e. conditional visibility of node

bool Controller::updateNodeDependencies(Node *node, bool force_update)
      {
      bool result = false;
      if (!node && !force_update)
            return result;
      if (LHRHMethod && (force_update || node == LHRHMethod)) {
            auto value = (MidiOperation::LHRHMethod)LHRHMethod->oper.value.toInt();
            switch (value) {
                  case MidiOperation::LHRHMethod::HAND_WIDTH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = false;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = false;
                        result = true;
                        break;
                  case MidiOperation::LHRHMethod::SPECIFIED_PITCH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = true;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = true;
                        result = true;
                        break;
                  }
            }
      if (LHRHdoIt && (force_update || node == LHRHdoIt)) {
            auto value = LHRHdoIt->oper.value.toBool();
            if (LHRHMethod)
                  LHRHMethod->visible = value;
            result = true;
            }
      if (searchTuplets && (force_update || node == searchTuplets)) {
            auto value = searchTuplets->oper.value.toBool();
            if (duplets)
                  duplets->visible = value;
            if (triplets)
                  triplets->visible = value;
            if (quadruplets)
                  quadruplets->visible = value;
            if (quintuplets)
                  quintuplets->visible = value;
            if (septuplets)
                  septuplets->visible = value;
            if (nonuplets)
                  nonuplets->visible = value;
            result = true;
            }

//      if (quantHuman) {
//            bool oneTrack = (trackCount == 1);
//            if (oneTrack != quantHuman->visible) {
//                  quantHuman->visible = oneTrack;
//                  result = true;
//                  }
//            }

      return result;
      }

} // namespace Ms
