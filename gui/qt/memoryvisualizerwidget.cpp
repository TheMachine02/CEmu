#include <QtGui/QPainter>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>

#include "utils.h"
#include "memoryvisualizerwidget.h"
#include "keypad/qtkeypadbridge.h"
#include "mainwindow.h"
#include "../../core/lcd.h"

MemoryVisualizerWidget::MemoryVisualizerWidget(QWidget *p) : QWidget(p) {
    refreshTimer = new QTimer(this);
    installEventFilter(keypadBridge);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MemoryVisualizerWidget::customContextMenuRequested, this, &MemoryVisualizerWidget::contextMenu);

    image = QImage(LCD_WIDTH, LCD_HEIGHT, QImage::Format_RGBX8888);
}

MemoryVisualizerWidget::~MemoryVisualizerWidget() {
    delete refreshTimer;
}

void MemoryVisualizerWidget::draw() {
    if (!guiEmuValid) {
        return;
    }

    lcd_setptrs(&data, &data_end, m_width, m_height, upbase, control, false);
    lcd_drawframe(image.bits(), data, data_end, control, size);
    update();
}

void MemoryVisualizerWidget::paintEvent(QPaintEvent*) {
    if (!guiEmuValid) {
        return;
    }

    QPainter c(this);
    const QRect& cw = c.window();

    c.setRenderHint(QPainter::SmoothPixmapTransform, cw.width() < static_cast<int>(m_width));
    c.drawImage(cw, image);
}

void MemoryVisualizerWidget::setRefreshRate(int rate) {
    if (!rate) {
        return;
    }
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(draw()));
    refreshTimer->stop();
    refreshTimer->setInterval(1000 / rate);
    refreshTimer->start();
    refresh = rate;
}

void MemoryVisualizerWidget::setConfig(uint32_t h, uint32_t w, uint32_t u, uint32_t c, uint32_t *d, uint32_t *e) {
    m_height = h;
    m_width = w;
    upbase = u;
    control = c;
    data = d;
    data_end = e;
    size = m_width * m_height;
    image = QImage(m_width, m_height, QImage::Format_RGBX8888);
}

void MemoryVisualizerWidget::contextMenu(const QPoint& posa) {
    QString coord = tr("Coordinate: ");
    QString copy_addr = tr("Copy Address");
    QString goto_mem = tr("Goto Memory View");
    QString toggle_write_watch = tr("Toggle Write Watchpoint");
    QString toggle_read_watch = tr("Toggle Read Watchpoint");
    QString toggle_rw_watch = tr("Toggle Read/Write Watchpoint");

    QTransform tr;
    tr.scale(m_width*1.0/width(), m_height*1.0/height());
    QPoint point = tr.map(posa);
    uint32_t x = point.x();
    uint32_t y = point.y();

    QString addr = int2hex(upbase + (x + (m_width * y)), 6);

    coord += QString::number(x) + QStringLiteral("x") + QString::number(y);
    copy_addr += QStringLiteral(" '") + addr + QStringLiteral("'");

    QMenu menu;
    menu.addAction(coord);
    menu.addSeparator();
    menu.addAction(copy_addr);
    //menu.addAction(goto_mem);
    //menu.addSeparator();
    //menu.addAction(toggle_write_watch);
    //menu.addAction(toggle_read_watch);
    //menu.addAction(toggle_rw_watch);

    QAction* item = menu.exec(mapToGlobal(posa));
    if (item) {
        if (item->text() == copy_addr) {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(addr.toLatin1());
        } else if (item->text() == goto_mem) {
            int count = 0;
            if (!guiDebug) {
                //MainWindow::debuggerChangeState();
            }
            while (!guiDebug && count < 20) {
                guiDelay(50);
                count++;
            }
            //MainWindow::memGoto(MEM_MEM, cursor.selectedText());
        } else if (item->text() == toggle_read_watch) {
            //MainWindow::watchpointAdd(watchpointNextLabel(), address, 1, DBG_READ_WATCHPOINT, true);
        } else if (item->text() == toggle_write_watch) {
            //MainWindow::watchpointAdd(watchpointNextLabel(), address, 1, DBG_WRITE_WATCHPOINT, true);
        } else if (item->text() == toggle_rw_watch) {
            //MainWindow::watchpointAdd(watchpointNextLabel(), address, 1, DBG_WRITE_WATCHPOINT | DBG_READ_WATCHPOINT, true);
        }
    }
}
