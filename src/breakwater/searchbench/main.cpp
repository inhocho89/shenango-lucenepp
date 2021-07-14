/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include "LuceneHeaders.h"
#include "Document.h"
#include "Field.h"
#include "Term.h"
#include "TermQuery.h"
#include "TopFieldDocs.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "KeywordAnalyzer.h"
#include "Query.h"
#include "cc/runtime.h"

using namespace Lucene;

constexpr uint64_t numDocs = 1000000;
constexpr uint64_t numWarmUpSamples = 10;
constexpr uint64_t numSamples = 100;
constexpr uint64_t NORM = 100;
constexpr int searchN = 1000;
constexpr int nworker = 1;

std::vector<String> terms;
std::vector<uint64_t> frequencies;
uint64_t weight_sum;
RAMDirectoryPtr dir;

void ReadFreqTerms() {
  std::cout << "Reading csv ...\t" << std::flush;
  std::ifstream fin("frequent_terms.csv");

  std::string line, word;
  String wword;
  uint64_t freq;

  weight_sum = 0;

  while(std::getline(fin, line)) {
    std::stringstream ss(line);
    
    getline(ss, word, ',');
    wword = String(word.length(), L' ');
    std::copy(word.begin(), word.end(), wword.begin());
    terms.push_back(wword);

    getline(ss, word, ',');
    freq = std::stoi(word) / NORM;
    frequencies.push_back(freq);
    weight_sum += freq;
  }

  fin.close();
  std::cout << "Done" << std::endl;
}

String ChooseTerm() {
  uint64_t rand_ = (uint64_t)rand() % weight_sum;
  int i;

  for(i = 0; i < frequencies.size(); ++i) {
    if (rand_ < frequencies[i]) {
      break;
    } else {
      rand_ -= frequencies[i];
    }
  }

  return terms[i];
}

DocumentPtr createDocument(const String& contents) {
  DocumentPtr document = newLucene<Document>();
  document->add(newLucene<Field>(L"contents", contents, Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
  return document;
}

void PopulateIndex() {
  std::cout << "Populating indices ...\t" << std::flush;
  uint64_t start = microtime();
  dir = newLucene<RAMDirectory>();

  IndexWriterPtr indexWriter = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true,
                                                      IndexWriter::MaxFieldLengthLIMITED);
  for (int i = 0 ; i < numDocs ; ++i) {
    indexWriter->addDocument(createDocument(ChooseTerm()));
  }
  uint64_t finish = microtime();
  std::cout << "Done (" << (finish - start) / 1000.0 << " ms)" << std::endl;

  std::cout << "Optimizing ...\t" << std::flush;
  start = microtime();
  indexWriter->optimize();
  finish = microtime();

  std::cout << "Done (" << (finish - start) / 1000.0 << " ms)" << std::endl;

  indexWriter->close();
}

void MeasureSearchTime(const String& term) {
  std::vector<float> latencies(numSamples * nworker);
  std::vector<rt::Thread> threads;

  for (int i = 0; i < nworker; ++i) {
    threads.emplace_back(rt::Thread([&, i]{
      IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
      QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"contents", term));
      uint64_t start,finish;

      for (int j = 0; j < numWarmUpSamples; ++j) {
        Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), searchN)->scoreDocs;
      }

      for (int j = 0; j < numSamples; ++j) {
        start = microtime();
        Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), searchN)->scoreDocs;
	finish = microtime();
	latencies[numSamples * i + j] = (finish - start) / 1000.0;
      }
    }));
  }

  for (int i = 0; i < nworker; ++i) {
    threads[i].Join();
  }

  std::sort(latencies.begin(), latencies.end());
  std::cout << "min = " << latencies[0] << ", "
	    << "p50 = " << latencies[numSamples * 0.5] << ", "
	    << "max = " << latencies[numSamples - 1] << std::endl;
}

void MainHandler(void *arg) {
  srand(time(NULL));
  // construct terms, frequencies vector
  ReadFreqTerms();

  // populate index
  PopulateIndex();

  MeasureSearchTime(L"coronavirus");  
}

int main(int argc, char* argv[]) {
  int ret;

  ret = runtime_init(argv[1], MainHandler, NULL);
  if (ret) {
    printf("failed to start runtime\n");
    return ret;
  }

  return 0;
}
