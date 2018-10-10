#include "mckay.hpp"
#include "trie.hpp"

template <typename List>
void initializeDisjointSetForestsOfPropagationList(List *pList) {
  for (auto iteratorCell = pList->sentinel->next;
       iteratorCell != pList->sentinel; iteratorCell = iteratorCell->next) {
    if (iteratorCell->value != CLASS_SENTINEL) {
      InheritedVertex *iVertex = (InheritedVertex *)iteratorCell->value;
      initializeDisjointSetForest(iVertex->equivalenceClassOfIsomorphism);
    }
  }

  return;
}

#define IS_DISCRETE_LIST (NULL)

template <typename T>
typename List__<T>::iterator firstNonTrivialCell(List__<T> *pList) {
  auto beginSentinel = pList->sentinel;
  auto endSentinel = beginSentinel;

  do {
    endSentinel = getNextSentinel(beginSentinel);

    if (beginSentinel->next->next != endSentinel) {
      return beginSentinel;
    }
    beginSentinel = endSentinel;
  } while (endSentinel != pList->sentinel);

  return IS_DISCRETE_LIST;
}

Order compareDiscretePropagationListOfInheritedVerticesWithAdjacentLabelsInner(
    InheritedVertex *iVertexA, InheritedVertex *iVertexB) {
  if (iVertexA == CLASS_SENTINEL && iVertexB == CLASS_SENTINEL) {
    return EQ;
  } else if (iVertexA == CLASS_SENTINEL && iVertexB != CLASS_SENTINEL) {
    CHECKER("CLASS_SENTINEL is invalid\n");
    exit(EXIT_FAILURE);
  } else if (iVertexA != CLASS_SENTINEL && iVertexB == CLASS_SENTINEL) {
    CHECKER("CLASS_SENTINEL is invalid\n");
    exit(EXIT_FAILURE);
  } else if (iVertexA->type < iVertexB->type) {
    return LT;
  } else if (iVertexA->type > iVertexB->type) {
    return GT;
  } else if (strcmp(iVertexA->name, iVertexB->name) < 0) {
    return LT;
  } else if (strcmp(iVertexA->name, iVertexB->name) > 0) {
    return GT;
  } else if (numStack(iVertexA->conventionalPropagationMemo) <
             numStack(iVertexB->conventionalPropagationMemo)) {
    return LT;
  } else if (numStack(iVertexA->conventionalPropagationMemo) >
             numStack(iVertexB->conventionalPropagationMemo)) {
    return GT;
  } else {
    int degree = numStack(iVertexA->conventionalPropagationMemo);
    int i;
    std::vector<int> *iStackA = iVertexA->conventionalPropagationMemo;
    std::vector<int> *iStackB = iVertexB->conventionalPropagationMemo;

    for (i = 0; i < degree; i++) {
      if (readStack(iStackA, i) < readStack(iStackB, i)) {
        return LT;
      } else if (readStack(iStackA, i) > readStack(iStackB, i)) {
        return GT;
      }
    }

    return EQ;
  }
}

Order compareDiscretePropagationListOfInheritedVerticesWithAdjacentLabelsInnerCaster(
    void *iVertexA, void *iVertexB) {
  return compareDiscretePropagationListOfInheritedVerticesWithAdjacentLabelsInner(
      (InheritedVertex *)iVertexA, (InheritedVertex *)iVertexB);
}

void initializeInheritedVertexAdjacentLabels(InheritedVertex *iVertex) {
  if (iVertex == CLASS_SENTINEL) {
    return;
  } else {
    iVertex->conventionalPropagationMemo->clear();

    return;
  }
}

void initializeInheritedVertexAdjacentLabelsCaster(void *iVertex) {
  initializeInheritedVertexAdjacentLabels((InheritedVertex *)iVertex);

  return;
}

void freeInheritedVertexOfPreserveDiscretePropagationList(
    InheritedVertex *iVertex) {
  if (iVertex != CLASS_SENTINEL) {
    freeStack(iVertex->conventionalPropagationMemo);
    free(iVertex);
  }

  return;
}

void freeInheritedVertexOfPreserveDiscretePropagationListCaster(void *iVertex) {
  freeInheritedVertexOfPreserveDiscretePropagationList(
      (InheritedVertex *)iVertex);

  return;
}

template <typename List>
void freePreserveDiscreteProapgationList(List *pdpList) {
  freeListWithValues(
      pdpList, freeInheritedVertexOfPreserveDiscretePropagationListCaster);

  return;
}

void freePreserveDiscreteProapgationListCaster(void *pdpList) {
  freePreserveDiscreteProapgationList((List__<void *> *)pdpList);

  return;
}

template <typename List>
Bool insertDiscretePropagationListOfInheritedVerticesWithAdjacentLabelToTable(
    RedBlackTree__<List *, List *>
        *discretePropagationListsOfInheritedVerticesWithAdjacentLabels,
    List *dpList, ConvertedGraph *cAfterGraph, int gapOfGlobalRootMemID) {
  Bool isExisting;

  putLabelsToAdjacentVertices(dpList, cAfterGraph, gapOfGlobalRootMemID);
  List *preserveDPList = copyListWithValues(dpList, copyInheritedVertexCaster);
  forEachValueOfList(dpList, initializeInheritedVertexAdjacentLabelsCaster);

  auto key = makeDiscretePropagationListKey(preserveDPList);
  List *seniorDPList = searchRedBlackTree(
      discretePropagationListsOfInheritedVerticesWithAdjacentLabels, key);

  if (seniorDPList == NULL) {
    insertRedBlackTree(
        discretePropagationListsOfInheritedVerticesWithAdjacentLabels, key,
        preserveDPList);
    isExisting = FALSE;
    return isExisting;
  } else {
    auto iteratorCell = preserveDPList->sentinel->next;
    auto iteratorCellSenior = seniorDPList->sentinel->next;

    while (iteratorCell != preserveDPList->sentinel) {
      if (iteratorCell->value != CLASS_SENTINEL) {
        InheritedVertex *iVertex = (InheritedVertex *)iteratorCell->value;
        InheritedVertex *iVertexSenior =
            (InheritedVertex *)iteratorCellSenior->value;

        unionDisjointSetForest(iVertex->equivalenceClassOfIsomorphism,
                               iVertexSenior->equivalenceClassOfIsomorphism);
      }

      iteratorCell = iteratorCell->next;
      iteratorCellSenior = iteratorCellSenior->next;
    }

    freePreserveDiscreteProapgationList(preserveDPList);

    isExisting = TRUE;
    return isExisting;
  }
}

template <typename List> void discretePropagationListDump(List *dpList) {
  listDump(dpList, inheritedVertexDumpCaster);
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");

  return;
}

void discretePropagationListDumpCaster(void *dpList) {
  discretePropagationListDump((List__<void *> *)dpList);

  return;
}

template <typename T>
Bool isNewSplit(typename List__<T>::iterator sentinelCell,
                typename List__<T>::iterator splitCell) {
  for (auto iteratorCell = sentinelCell->next; iteratorCell != splitCell;
       iteratorCell = iteratorCell->next) {
    InheritedVertex *splitIVertex = (InheritedVertex *)splitCell->value;
    InheritedVertex *iteratorIVertex = (InheritedVertex *)iteratorCell->value;

    if (isInSameDisjointSetForest(
            splitIVertex->equivalenceClassOfIsomorphism,
            iteratorIVertex->equivalenceClassOfIsomorphism)) {
      return FALSE;
    }
  }

  return TRUE;
}

template <typename T>
Bool listMcKayInner(
    List__<T> *propagationListOfInheritedVertices, ConvertedGraph *cAfterGraph,
    int gapOfGlobalRootMemID,
    RedBlackTree__<List__<T> *, List__<T> *>
        *discretePropagationListsOfInheritedVerticesWithAdjacentLabels) {
  Bool isUsefulBranch = TRUE;

  auto stabilizer = copyList(propagationListOfInheritedVertices);
  getStableRefinementOfConventionalPropagationList(stabilizer, cAfterGraph,
                                                   gapOfGlobalRootMemID);

  /*
  CHECKER("###### after stable refinement ######\n");
  listDump(stabilizer,inheritedVertexDumpCaster),fprintf(stdout,"\n");
  //*/

  auto beginSentinel = firstNonTrivialCell(stabilizer);

  if (beginSentinel == IS_DISCRETE_LIST) {
    isUsefulBranch =
        !insertDiscretePropagationListOfInheritedVerticesWithAdjacentLabelToTable(
            discretePropagationListsOfInheritedVerticesWithAdjacentLabels,
            stabilizer, cAfterGraph, gapOfGlobalRootMemID);
  } else {
    Bool isFirstLoop = TRUE;

    auto endSentinel = getNextSentinel(beginSentinel);
    auto sentinelCell = new typename List__<T>::iterator(CLASS_SENTINEL);
    insertNextCell(beginSentinel, sentinelCell);

    for (auto iteratorCell = sentinelCell; iteratorCell->next != endSentinel;
         iteratorCell = iteratorCell->next) {
      auto splitCell = iteratorCell->next;

      if (isNewSplit(sentinelCell, splitCell)) {
        cutCell(splitCell);
        insertNextCell(beginSentinel, splitCell);

        Bool isUsefulChild = listMcKayInner(
            stabilizer, cAfterGraph, gapOfGlobalRootMemID,
            discretePropagationListsOfInheritedVerticesWithAdjacentLabels);

        cutCell(splitCell);
        insertNextCell(iteratorCell, splitCell);

        if (isFirstLoop) {
          isFirstLoop = FALSE;
          if (!isUsefulChild) {
            isUsefulBranch = FALSE;
            break;
          } else {
            isUsefulBranch = TRUE;
          }
        }
      }
    }
  }

  freeList(stabilizer);

  return isUsefulBranch;
}

template <typename List>
List *listMcKay(List *propagationListOfInheritedVertices,
                ConvertedGraph *cAfterGraph, int gapOfGlobalRootMemID) {
  if (propagationListOfInheritedVertices->empty()) {
    List *canonicalDiscreteRefinement =
        copyList(propagationListOfInheritedVertices);
    return canonicalDiscreteRefinement;
  } else {
    initializeDisjointSetForestsOfPropagationList(
        propagationListOfInheritedVertices);
    auto discretePropagationListsOfInheritedVerticesWithAdjacentLabels =
        new RedBlackTree__<List *, List *>();

    classifyConventionalPropagationListWithAttribute(
        propagationListOfInheritedVertices, cAfterGraph, gapOfGlobalRootMemID);

    /*
    CHECKER("###### after attribute classifying ######\n");
    listDump(propagationListOfInheritedVertices,inheritedVertexDumpCaster),fprintf(stdout,"\n");
    //*/

    listMcKayInner(
        propagationListOfInheritedVertices, cAfterGraph, gapOfGlobalRootMemID,
        discretePropagationListsOfInheritedVerticesWithAdjacentLabels);

    List *canonicalDiscreteRefinement = (List *)copyListWithValues(
        minimumElementOfRedBlackTree(
            discretePropagationListsOfInheritedVerticesWithAdjacentLabels),
        copyInheritedVertexCaster);

    /*
    CHECKER("########### candidates of canonical discrete refinement
    ###########\n");
    redBlackTreeValueDump(discretePropagationListsOfInheritedVerticesWithAdjacentLabels,discretePropagationListDumpCaster);
    //*/

    freeRedBlackTreeWithValue(
        discretePropagationListsOfInheritedVerticesWithAdjacentLabels,
        freePreserveDiscreteProapgationListCaster);

    return canonicalDiscreteRefinement;
  }
}

template <typename List>
Bool checkIsomorphismValidity(DynamicArray *slimKeyCollection,
                              RedBlackTree *McKayKeyCollection,
                              List *canonicalDiscreteRefinement, int stateID) {
  Bool isValid = TRUE;

  if (stateID != 0) {
    auto key = makeDiscretePropagationListKey(canonicalDiscreteRefinement);
    CollectionInt seniorID =
        (CollectionInt)searchRedBlackTree(McKayKeyCollection, key) - 1;
    if (seniorID != -1) {
      if (stateID != seniorID) {
        fprintf(stdout, "stateID is wrong.\n");
        fprintf(stdout, "juniorStateID is %d\n", stateID);
        fprintf(stdout, "seniorStateID is %d\n", seniorID);
        isValid = FALSE;
        return isValid;
      }
    } else {
      insertRedBlackTree(McKayKeyCollection, key, (void *)(stateID + 1));
    }

    List *seniorDiscreteRefinement =
        (List *)readDynamicArray(slimKeyCollection, stateID);
    if (seniorDiscreteRefinement != NULL) {
      if (compareDiscretePropagationListOfInheritedVerticesWithAdjacentLabels(
              canonicalDiscreteRefinement, seniorDiscreteRefinement) != EQ) {
        printf("adjacency list is wrong.\n");
        isValid = FALSE;
        return isValid;
      } else {
        freePreserveDiscreteProapgationList(canonicalDiscreteRefinement);
      }
    } else {
      writeDynamicArray(slimKeyCollection, stateID,
                        canonicalDiscreteRefinement);
    }
  }

  return isValid;
}

template <typename List>
List *trieMcKay(Trie *trie, DiffInfo *diffInfo, Graphinfo *cAfterGraph,
                Graphinfo *cBeforeGraph) {
  int gapOfGlobalRootMemID =
      cBeforeGraph->globalRootMemID - cAfterGraph->globalRootMemID;
  int stepOfPropagation;
  Bool verticesAreCompletelySorted =
      triePropagate(trie, diffInfo, cAfterGraph, cBeforeGraph,
                    gapOfGlobalRootMemID, &stepOfPropagation);
  if (IS_DIFFERENCE_APPLICATION_MODE && verticesAreCompletelySorted) {
    /* printf("%s:%d\n", __FUNCTION__, __LINE__); */
    return makeList();
  } else {

    List *propagationListOfInheritedVertices =
        makeConventionalPropagationList(trie, stepOfPropagation);

    /*
       CHECKER("###### before list propagate ######\n");

    //*/
    /* printf("%s:%d\n", __FUNCTION__, __LINE__); */
    /* listDump(propagationListOfInheritedVertices,inheritedVertexDumpCaster),fprintf(stdout,"\n");
     */
    List *canonicalDiscreteRefinement =
        listMcKay(propagationListOfInheritedVertices, cAfterGraph->cv,
                  gapOfGlobalRootMemID);

    /*
       CHECKER("###### after list propagate ######\n");
       listDump(canonicalDiscreteRefinement,inheritedVertexDumpCaster),fprintf(stdout,"\n");
    //*/

    freeList(propagationListOfInheritedVertices);

    return canonicalDiscreteRefinement;
  }
}