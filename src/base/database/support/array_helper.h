#ifndef ARRAY_HELPER_H
#define ARRAY_HELPER_H

#include <QVector>
#include <QList>
#include <vector>
#include <map>
#include <functional>

namespace Arr
{

// Checks if a value exists in an array
template <typename T>
bool in_array(const T &needle, const std::vector<T> &haystack)
{
  for (const T &item : haystack) {
    if (needle == item) return true;
  }
  return false;
}

template <typename T>
bool in_array(const T &needle, const QVector<T> &haystack)
{
    for (const T &item : haystack) {
        if (needle == item) return true;
    }
    return false;
}

template <typename T>
bool in_array(const T &needle, const QList<T> &haystack)
{
    for (const T &item : haystack) {
        if (needle == item) return true;
    }
    return false;
}

// Filters elements of an array using a callback function.
template <typename T>
std::vector<T> array_filter(const std::vector<T> &array, std::function<bool(const T&)> callback)
{
  std::vector<T> result;
  for (const T &item : array)
  {
    if (callback(item))
      result.push_back(item);
  }
  return result;
}

template <typename T>
QList<T> array_filter(const QList<T> &array, std::function<bool(const T&)> callback)
{
    QList<T> result;
    for (const T &item : array)
    {
        if (callback(item))
            result.push_back(item);
    }
    return result;
}

template <typename T>
QVector<T> array_filter(const QVector<T> &array, std::function<bool(const T&)> callback)
{
    QVector<T> result;
    for (const T &item : array)
    {
        if (callback(item))
            result.push_back(item);
    }
    return result;
}

// Merge one or more arrays.
template <typename T_k, typename T_v>
std::map<T_k, T_v> array_merge(const std::map<T_k, T_v> &array1, const std::map<T_k, T_v> &array2)
{
  std::map<T_k, T_v> result;
  for (auto it = array1.begin(); it != array1.end(); ++it)
    result[it->first] = it->second;
  for (auto it = array2.begin(); it != array2.end(); ++it)
    result[it->first] = it->second;

  return result;
}

} // namespace

#endif // ARRAY_HELPER_H
