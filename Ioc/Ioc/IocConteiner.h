#ifndef __IOC_ICOCONTAINER_H__
#define __IOC_ICOCONTAINER_H__

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

class Any;	//ָ����������,ʹ��boost���Any<boost/any.hpp>
class IocContainer
{
public:
	IocContainer() {};
	~IocContainer() {};

public:
	//ע����Ҫ��������Ĺ��캯������Ҫ����һ��Ψһ�ı�ʶ�����Ա��ں��洴������ʱ������ҡ�
	template<typename T, typename Drived, typename... Args>
	void RegisterType(const std::string& strKey)
	{
		//ͨ���հ������˲�������
		std::function<T*(Args...)> function = [Args... args] {return new Drived(args...); };
		RegisterType(strKey, function);
	}

	template<typename T, typename...Args>
	void RegisterTypeSimple(const std::string& strKey)
	{
		std::function<T*(Args...)> function = [Args... args]{ return new T(args...); };
		RegisterType(strKey, function);
	}

	//����Ψһ��ʶȥ���Ҷ�Ӧ�Ĺ�������������ָֻ�����
	template<class T, typename... Args>
	T* Resolve(const std::string& strKey, Args... args)
	{
		if (m_creatorMap.find(strKey) == m_creatorMap.end())
			return nullptr;

		Any resolver = m_creatorMap[strKey];
		std::function<T* (Args...)> function = resolver.AnyCast<std::function<T* (Args...)> >();
		return function(args...);
	}

	//��������ָ�����
	template<class T, typename... Args>
	std::shared_ptr<T> ResolverShared(const std::string& strKey, Args... args)
	{
		T* ptr = Resolve(strKey, args);
		return std::shared_ptr<T>(ptr);
	}

private:
	void RegisterType(const std::string& strKey, Any creator)
	{
		if (m_creatorMap.find(strKey) != m_creatorMap.end())
			throw std::invalid_argument("Already exist");

		m_creatorMap.emplace(strKey, creator);
	}

private:
	std::unordered_map<std::string, Any> m_creatorMap;
};

#endif	//__IOC_ICOCONTAINER_H__
