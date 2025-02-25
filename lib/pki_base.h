/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2001 - 2020 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#ifndef __PKI_BASE_H
#define __PKI_BASE_H

#include <QString>
#include <QRegularExpression>
#include <QVariant>
#include <QByteArray>
#include <QPalette>
#include <QBrush>
#include "BioByteArray.h"
#include "asn1time.h"
#include "pkcs11_lib.h"
#include "base.h"
#include "pki_lookup.h"
#include "headerlist.h"
#include "sql.h"
#include "xfile.h"
#include "pki_export.h"

#define pki_openssl_error() _openssl_error(*this, __FILE__, __LINE__)
#define pki_ign_openssl_error() _ign_openssl_error(*this, __FILE__, __LINE__)

enum pki_source {
	unknown,
	imported,
	generated,
	transformed,
	token,
	legacy_db,
	renewed
};

#define VIEW_item_id 0
#define VIEW_item_name 1
#define VIEW_item_type 2
#define VIEW_item_date 3
#define VIEW_item_source 4
#define VIEW_item_comment 5

extern pki_lookup Store;

class pki_base : public QObject
{
		Q_OBJECT

	public: /* static */
		static QRegularExpression limitPattern;
		static QString rmslashdot(const QString &fname);
		static unsigned hash(const QByteArray &ba);
		static bool pem_comment;
		static int count;
		static void setupColors(const QPalette &pal);

	protected:
		QVariant sqlItemId{};
		QString desc{}, comment{};
		a1time insertion_date{};
		enum pki_type pkiType{ none };
		/* model data */
		pki_base *parent{};
		QString filename{};
		QList<pki_base*> childItems{};
		mutable QRegularExpression lastPattern{};
		int iamvisible{ 1 };
		static QBrush red, yellow, cyan;

		void my_error(const QString &error) const;
		virtual QByteArray PEM_comment() const;
		virtual void collect_properties(QMap<QString, QString> &) const;

	public:
		enum msg_type {
			msg_import,
			msg_delete,
			msg_delete_multi,
			msg_create,
		};
		enum print_opt {
			print_openssl_txt,
			print_pem,
			print_coloured,
		};
		enum pki_source pkiSource{ unknown };

		pki_base(const QString &d = QString());
		pki_base(const pki_base *p);
		virtual ~pki_base();

		QList<pki_base*> getChildItems() const;
		void clear();
		void recheckVisibility()
		{
			lastPattern = QRegularExpression();
		}
		QString getIntName() const
		{
			return desc;
		}
		void setFilename(const QString &s)
		{
			recheckVisibility();
			filename = s;
		}
		QString getFilename() const
		{
			return filename;
		}
		void inheritFilename(pki_base *pki) const
		{
			pki->setFilename(getFilename());
		}
		virtual QString comboText() const;
		virtual void print(BioByteArray &b, enum print_opt opt) const;
		QString getUnderlinedName() const;
		void setIntName(const QString &d)
		{
			recheckVisibility();
			desc = d;
		}
		virtual void autoIntName(const QString &file);
		QString getComment() const
		{
			return comment;
		}
		void setComment(const QString &c)
		{
			recheckVisibility();
			comment = c;
		}
		QVariant getSqlItemId() const
		{
			return sqlItemId;
		}
		enum pki_type getType() const
		{
			return pkiType;
		}
		QString getTypeString() const;
		QString i2d_b64() const
		{
			return QString::fromLatin1(i2d().toBase64());
		}
		a1time getInsertionDate() const
		{
			return insertion_date;
		}
		virtual QByteArray i2d() const;
		virtual bool compare(const pki_base *) const;
		virtual QString getMsg(msg_type msg) const;
		virtual const char *getClassName() const;

		/* Tree View management */
		void setParent(pki_base *p);
		pki_base *getParent() const;
		pki_base *child(int row);
		void insert(pki_base *item);
		int childCount() const;
		void takeChild(pki_base *pki);
		pki_base *takeFirst();
		int indexOf(const pki_base *child) const;

		/* Token handling */
		virtual void deleteFromToken();
		virtual void deleteFromToken(const slotid &);
		virtual int renameOnToken(const slotid &, const QString &);

		/* Import / Export management */
		virtual bool pem(BioByteArray &b);
		virtual void fromPEM_BIO(BIO *, const QString &);
		virtual void fromPEMbyteArray(const QByteArray &, const QString &);
		virtual void fload(const QString &);
		virtual void writeDefault(const QString&) const;

		/* Qt Model-View methods */
		virtual QVariant bg_color(const dbheader *) const
		{
			return QVariant();
		}
		virtual QVariant column_data(const dbheader *hd) const;
		virtual QVariant getIcon(const dbheader *hd) const;
		virtual QVariant column_tooltip(const dbheader *hd) const;
		virtual a1time column_a1time(const dbheader *hd) const;
		virtual bool visible() const;
		int isVisible();
		bool childVisible() const;

		/* SQL management methods */
		QSqlError insertSql();
		virtual QSqlError insertSqlData()
		{
			return QSqlError();
		}
		QSqlError deleteSql();
		virtual QSqlError deleteSqlData()
		{
			return QSqlError();
		}
		virtual void restoreSql(const QSqlRecord &rec);
		QSqlError sqlItemNotFound(QVariant sqlId) const;
		unsigned hash() const;
		QString pki_source_name() const;
		QString get_dump_filename(const QString &dirname,
					  const QString &ext) const;
		void selfComment(QString msg);
		QStringList icsVEVENT(const a1time &expires,
		    const QString &summary, const QString &description) const;
		operator QString() const
		{
			return QString("(%1[%2]:%3)").arg(getClassName())
				.arg(getSqlItemId().toString()).arg(getIntName());
		}
};

Q_DECLARE_METATYPE(pki_base *);
#endif
