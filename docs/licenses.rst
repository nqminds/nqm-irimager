========
Licenses
========

Source-code licenses
====================

The following licenses are taken from the ``LICENSES/`` folder and represent
the licenses that subsets of the source-code are available under.

Please see each individual file for their ``# SPDX-License-Identifier:`` and
``# SPDX-FileCopyrightText`` SPDX tags to see which files these licenses cover.

``LICENSES/MIT.txt``
--------------------

.. include:: ../LICENSES/MIT.txt
   :literal:

``LICENSES/BSD-3-Clause.txt``
-----------------------------

.. include:: ../LICENSES/BSD-3-Clause.txt
   :literal:

Binary licenses
===============

The following licenses apply to the built binaries, due to static/dynamic
linking.

libirmager
----------

The IRImagerDirect SDK library is used under the **BSD-2-Clause** license, see
`<http://ftp.evocortex.com/freebsd.txt>`_.

  Copyright (c) 2016-2017, Optris GmbH / Evocortex GmbH
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are
  those of the authors and should not be interpreted as representing official
  policies, either expressed or implied, of the FreeBSD Project.

libudev
-------

libudev is dynamically linked, as it is required by the IRImagerDirect SDK.

Some udev sources are licensed under **GPL-2.0-or-later**, so the
udev binaries as a whole are also distributed under **GPL-2.0-or-later**.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
  USA.

jbcoe's propagate const
-----------------------

`jbcoe`_'s https://github.com/jbcoe/propagate_const header is used under the
**MIT** license.

.. _jbcoe: https://github.com/jbcoe

  Copyright (c) 2017 Jonathan B. Coe

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

Eigen
-----

The `Eigen`_ C++ template library for linear algebra is used under the
**MPL-2.0** license.

.. _Eigen: https://eigen.tuxfamily.org/index.php?title=Main_Page

  This Source Code Form is subject to the terms of the
  Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
  this file, You can obtain one at http://mozilla.org/MPL/2.0/.
